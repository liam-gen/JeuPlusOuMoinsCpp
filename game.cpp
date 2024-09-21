/*
* Liam CHARPENTIER
* Jeu plus ou moins
* 
* Le programme charge mal les caractères utf8 et les ascii (pour tout ce qui est gras, souligné, en couleurs) sur codeblocks.
* Cependant il fonctionne bien sur Visual Studio et l'executable sur le github les affiches bien.
*/

using namespace std;

#include <iostream>
#include <random>
#include <string>
#include <windows.h>
#include <chrono>
#include <map>
#include <list>

// Accéder à system() pour vider la console
#include <cstdlib>

class Style
{
private:
    map<string, string> styles = {
        {"green", "\033[32m"},
        {"blue", "\033[34m"},
        {"red", "\033[31m"},
        {"bold", "\033[1m"},
        {"underline", "\033[4m"},
    };
public: 
    // Récupère et affiche le texte donnée avec du style (gras, couleurs, ...)
    string get(string data, string color)
    {
        if (styles.find(color) != styles.end()) {
            return styles[color] + data + "\033[0m";
        }
        else {
            return data;
        }
    }
};

// Classe du joueur (nom, score)
class Player {
public:
    string username;
    int score = 0;

    void setScore(int score)
    {
        this->score = score;
    }

    void addScore()
    {
        this->score += 1;
    }

    void setUsername(string username)
    {
        this->username = username;
    }
};

// Un paramètre a un id (ex: min), un nom "d'affichage" (ex: Nombre maximum) et une valeur (ex: 0)
struct Setting {
    string id;
    string name;
    int value;
};

class Settings {

    public:
        vector<Setting> settings;

        // Définir les paramètres et les valeurs par défaut
        Settings() {
            settings.push_back({ "min", "Nombre minimum", 0 });
            settings.push_back({ "max", "Nombre maximum", 100 });
        }

        // Récupérer un paramètre par son id
        int get(const string& id) {
            for (const auto& setting : settings) {
                if (setting.id == id) {
                    return setting.value;
                }
            }
            return -1;
        }

        // Récupérer le nom d'un paramètre par son id
        string getName(const string& id) {
            for (const auto& setting : settings) {
                if (setting.id == id) {
                    return setting.name;
                }
            }
        }

        // Def la valeur d'un paramètre par son id
        void setValue(const string& id, int valeur) {
            for (auto& setting : settings) {
                if (setting.id == id) {
                    setting.value = valeur;
                    return;
                }
            }
        }
};

// Sauvegarder en mémoire les parties précédentes (joueurs, durée, nombre à trouver, score)

// TOFIX: Sauvegarder en fichier plutôt qu'en mémoire
class GameSession {
    public:
        vector<Player> players;
        double duration = 0;
        int number = 0;
        int score = 0;
};

class Game
{
    private:
        // Charger les utilisateurs, les sessions, le temps de démarage et les paramètres
        Style style = Style();
        vector<Player> players;
        vector<GameSession> games;
        chrono::high_resolution_clock::time_point startTime;
        Settings settings = Settings();

        // récupérer la partie dans celles stockées avec le meilleur temps et le moins de tentatives

        GameSession bestTentatives() const {

            auto minSession = this->games.begin();
            for (auto it = this->games.begin(); it != this->games.end(); ++it) {
                if (it->score < minSession->score) {
                    minSession = it;
                }
                
            }

            return *minSession;
        }

        GameSession bestDuration() const {

            auto minSession = this->games.begin();
            for (auto it = this->games.begin(); it != this->games.end(); ++it) {
                if (it->duration < minSession->duration) {
                    minSession = it;
                }
            }

            return *minSession;
        }
        

   public:
       // Nombre à faire deviner
       int number = this->generateRandomNumber(this->settings.get("min"), this->settings.get("max"));

       // Pour la boucle while (tant que la partie est pas finie)
       int ended = false;

   void askForUsers() {
       // Récupérer le nombre de joueurs
       this->show("Entrez le nombre de joueurs : ", false);
       int nbPlayers = 2;
       cin >> nbPlayers;

       // Si il y a un problème (pas int ou pas sup à 2) on ignore l'entrée et on redemande
       if (nbPlayers < 2 || cin.fail())
       {
           cin.clear();
           cin.ignore(1000, '\n');
           this->show("Vous devez entrer un nombre de joueurs supérieur ou égal à 2 !");
           return this->askForUsers();
       }

       this->initPlayers(nbPlayers);
   };


    void initGame(bool multiplayers = false)
    {
        // Remettre tout à 0 (nombre de joeurs, nombre, console, ...) et tout charger
        this->ended = false;
        this->number = this->generateRandomNumber(this->settings.get("min"), this->settings.get("max"));

        this->players.clear();

        this->clear();
        this->start();


        // Si il y a plusieurs joeurs, on leur demande leurs pseudos
        if (multiplayers) {

            askForUsers();

            this->clear();
            this->start();
        }

        this->loadMessage();

        // On stocke le début du chrono
        this->startTime = chrono::high_resolution_clock::now();
    }

    

    int generateRandomNumber(int min, int max){

        // Trouver un nombre aléatoire avec le package random
        // TOFIX : Comprendre comment/pourquoi ça fonctionne 😭
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distr(min, max);

        return distr(gen);
    }

    void clear() {
        // Vider la console
        system("cls");
    }

    void check(int input, Player* player)
    {
        // Si le nombre est le bon
        if (input == this->number)
        {
            // ON ajoute 1 tentative
            player->addScore();

            // Affichage + fin de boucle + chargement du temps de fin
            this->show(this->style.get("Bravo !", "green"));
            this->ended = true;

            auto endTime = chrono::high_resolution_clock::now();

            chrono::duration<double> duration = endTime - this->startTime;
            this->show("Durée de la partie : "+to_string(duration.count())+"s");

            // TOFIX: Saucegarder dans un fichier plutot que la mémoire
            GameSession storeGame = GameSession();
            storeGame.duration = duration.count();
            storeGame.number = this->number;
            storeGame.score = player->score;

            // Afficher + stocker les données des joueurs
            if (this->players.size() > 1) {
                for (int i = 0; i < this->players.size(); i++) {
                    this->show(this->players[i].username + " : " + to_string(this->players[i].score) + " tentatives");
                    storeGame.players.emplace_back(this->players[i]);
                }
            }
            else {
                this->show("Réussi en "+to_string(this->players[0].score)+ " tentatives");
                storeGame.players.emplace_back(this->players[0]);
            }

            
            // Stocker cette partie
            this->games.emplace_back(storeGame);

            this->show("\n\n");
            this->showMenu();
        }
        else if (input < this->number)
        {
            this->show(this->style.get("C'est plus (+)", "blue"));

            player->addScore();
        }
        else
        {
            this->show(this->style.get("C'est moins (-)", "red"));

            player->addScore();
        }
    }

    void show(string data, bool breakLine = true)
    {
        breakLine ? cout << data << endl : cout << data;
    }

    void showMultiple(list<string> data)
    {
        // Afficher plusieurs lignes en 1 fonction parce que 50 show c'est pas bo
        for (string line : data) {
            cout << line << endl;
        }
        
    }

    int askForNumberCin()
    {
        int x;
        cin >> x;
        
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(1000, '\n');
            this->show("Vous devez entrer un nombre entre "+to_string(this->settings.get("min"))+" et " + to_string(this->settings.get("max")) + " !\nRéssayez : ", false);
            return this->askForNumberCin();
        }

        return x;
    }

    void askForNumber(Player* player)
    {
        // Charger le prochain joueur de demander le nombre
        if (this->players.size() > 1)
        {
            this->show("Au tour de " + player->username + ", entre un nombre : ", false);

            this->check(this->askForNumberCin(), player);
        }
        else
        {
            this->show("Entrez un nombre : ", false);
            this->check(this->askForNumberCin(), player);
        }
        
    }

    void start()
    {
        // "Logo" de démarrage
        this->show(R"(
       _   ______   _    _                ____    _    _            
      | | |  ____| | |  | |      _       / __ \  | |  | |           
      | | | |__    | |  | |    _| |_    | |  | | | |  | |    ______ 
  _   | | |  __|   | |  | |   |_   _|   | |  | | | |  | |   |______|
 | |__| | | |____  | |__| |     |_|     | |__| | | |__| |           
  \____/  |______|  \____/               \____/   \____/            
                                                                    
                                                                    
)");
    }

    void showMenu()
    {
        // Afficher le menu
        this->showMultiple({
            this->style.get(this->style.get("Menu", "bold"), "underline"),
            "======================",
            "1. Afficher les règles du jeu",
            "2. Afficher les parties précédentes",
            "3. Lancer le jeu en solo",
            "4. Lancer le jeu à plusieurs",
            "5. Paramètres",
            "6. Informations"
        });

        // Attendre une entrée utilisateur
        this->show("\nEntrez une action : ", false);


        int x;
        cin >> x;

        // Lancer la commande en fonction de l'entrée
        switch (x)
        {
            case 1:
                displayRules();
                break;
            case 2:
                previousGames();
                break;
            case 3:
                startGame();
                break;
            case 4:
                startGameMulti();
                break;
            case 5:
                startSettings();
                break;
            case 6:
                startInfos();
                break;
            default:
                cin.clear();
                cin.ignore(1000, '\n');
                this->clear();
                this->start();
                this->showMenu();
                break;
        };
    }

    void startInfos()
    {
        // Afficher les infos du jeu
        this->clear();
        this->start();

        this->showMultiple({
            this->style.get("Informations", "bold"),
            "======================",
            "GitHub : https://github.com/liam-gen/plus-or-minus-game",
            "\n\n"
        });

        this->showMenu();
    }

    void startSettings() {
        // Afficher les paramètres du jeu
        this->clear();
        this->start();

        this->show(this->style.get("Paramètres du jeu", "bold"));
        this->show("======================");
        // Récupérer tous les paramètres de la classe Settings
        for (const auto& setting : this->settings.settings) {
            this->show(this->style.get(setting.name, "bold") + " : " + to_string(setting.value));
        }
        this->show("\n\n");

        this->showSettingsMenu();
    }

    void showSettingsMenu()
    {
        // Afficher le menu de séléction de paramètres
        this->show(this->style.get(this->style.get("Menu", "bold"), "underline"));
        this->show("======================");
        this->show("1. Retourner au menu");

        // En gros z = 2 car 1 c'est pour lancer le menu (le fait de changer un paramètre commence à 2)
        int z = 2;
        for (const auto& setting : this->settings.settings) {
            this->show(to_string(z)+". Changer la valeur de \""+this->style.get(setting.name, "bold") + "\"");
            z++;
        }

        this->show("\nEntrez une action : ", false);

        int x = 0;
        cin >> x;

        if (x == 1)
        {
            this->clear();
            this->start();
            this->showMenu();
        }
        else {
            // x - 2 car on commence à 2 pour le menu alors que l'index des paramètres commence à 0
            this->askForChangeSetting(x - 2);
        }

        
    }

    void askForChangeSetting(int index) {

        // Si l'index est pas bon on quitte et recharge les paramtres 
        if (index < 0 || index >= this->settings.settings.size()) {
            this->startSettings();
        }

        Setting& setting = this->settings.settings[index];

        cout << "\nChanger la valeur de \"" << setting.name << "\" : ";

        int x;
        cin >> x;

        // Si y a un problème avec l'entrée
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Vous devez entrer un nombre!\nRéessayez : ";
            return this->askForChangeSetting(index);
        }

        // Modifier la valeur
        
        this->settings.setValue(setting.id, x);

        cout << "La nouvelle valeur de \"" << setting.name << "\" est : " << setting.value << endl;

        this->startSettings();
    }

    void loadMessage()
    {
        // Lancer les messages de chargement
        string messages[] = { "Préparation du jeu", "Génération d'un nombre aléatoire", "Chargement... Normalement ça marche sur ma machine"};
        const int nombreDeMessages = sizeof(messages) / sizeof(messages[0]);
        int indexMessage = 0;

        // Pour chaque message
        for (int z = 0; z < nombreDeMessages; z++)
        {
            string message = messages[indexMessage];
            for (int x = 0; x < 4; x++)
            {
                // Afficher ., .., ... à la fin toutes les 400ms
                cout << "\r" << this->style.get(message, "bold") << flush;
                Sleep(400);
                message += ".";
            }
            indexMessage = (indexMessage + 1) % nombreDeMessages;
        }

        this->clear();
        this->start();
        
    }

    void displayRules()
    {
        // Afficher les règles
        this->clear();
        this->start();

        this->showMultiple({
            this->style.get("Règles du jeu", "bold"),
            "======================",

            this->style.get("Solo :", "bold"),
            "Le jeu choisis un nombre entre " + to_string(this->settings.get("min")) + " et " + to_string(this->settings.get("max")) + " puis il te demande quel nombre tu veux entrer.",
            "Tu entres un nombre et le jeu te dit si c'est + ou -.",
            "Tu gagnes lorsque tu as trouvé le nombre !\n",

            this->style.get("Multijoueurs :", "bold"),
            "Tu choisis un nombre de joueurs (a partir de 2).",
            "Vous entrez vos surnoms puis vous lancez la partie."
            "Le jeu choisis un nombre entre " + to_string(this->settings.get("min")) + " et " + to_string(this->settings.get("max")) + ".",
            "Vous jouez chacun votre tours (le jeu affiche le joueur)",
            "Le premier à trouver le nombre à gagné !",

            "\n\n"
        });

        this->showMenu();
    }

    void previousGames() {
        // Afficher les parties précédentes
        this->clear();
        this->start();

        this->show(this->style.get("Parties récentes", "bold"));
        this->show("======================");

        // Si aucune partie
        if (this->games.size() == 0)
        {
            this->show("Aucune partie récente :/");
        }
        else {
            // Récup des meilleures
            vector<GameSession> bestGames = { bestTentatives(), bestDuration() };

            vector<string> texts = { "Le moins de tentatives" ,  "Meilleur temps" };

            int value = 0;

            for (GameSession& game : bestGames)
            {
                
                this->show(this->style.get(texts[value], "green"));
                value++;
                if (game.players.size() == 1)
                {
                    this->show("Type : Solo");
                    this->show("Tentatives : " + to_string(game.players[0].score));
                }
                else
                {
                    this->show("Joueurs :");
                    for (int x = 0; x < game.players.size(); x++)
                    {
                        Player user = game.players[x];
                        this->show(user.username + " avec " + to_string(user.score) + " tentatives");
                    }
                }

                this->show("Durée : " + to_string(game.duration)+"s");
                this->show("Nombre : " + to_string(game.number));

                this->show("======================");
            }
            
            

            this->show("\n\n======================");


            for (int i = 0; i < this->games.size(); i++) {
                if (this->games[i].players.size() == 1)
                {
                    this->showMultiple({
                        "Type : Solo",
                        "Durée : " + to_string(this->games[i].duration) + "s",
                        "Nombre : " + to_string(this->games[i].number),
                        "Tentatives : " + to_string(this->games[i].players[0].score)
                    });
                }
                else {
                    this->showMultiple({
                        "Type : Multijoueurs",
                        "Durée : " + to_string(this->games[i].duration) + "s",
                        "Nombre : " + to_string(this->games[i].number),
                        "Joueurs : "
                    });

                    for (int x = 0; x < this->games[i].players.size(); x++)
                    {
                        Player user = this->games[i].players[x];
                        this->show(user.username + " avec " + to_string(user.score) + " tentatives");
                    }
                    
                }
                this->show("======================");
                
            }
        }

        this->show("\n\n");

        this->showMenu();
    }

    void startGame()
    {
        // Lancer le jeu en solo
        this->initGame();

        Player player = Player();
        this->players.emplace_back(player);

        while (!this->ended)
        {
            this->askForNumber(&this->players[0]);
        }
    }

    void initPlayers(int playersNb = 2)
    {
        // Récup le pseudo des joueurs
        for (int i = 0; i < playersNb; i++)
        {
            Player player = Player();

            this->show("Entrez le nom du joueur " + to_string(i + 1) + " : ", false);
            string username = "Joueur non nomé";
            cin >> username;

            player.setUsername(username);

            this->players.emplace_back(player);
        }
    }


    void startGameMulti()
    {
        // Démarer le jeu à plusieurs
        this->initGame(true);

        while (!this->ended)
        {
            for (int i = 0; i < this->players.size(); i++) {
                this->askForNumber(&players[i]);
            }
        }
    }
};

int main()
{
    // Accepter tout ce qui est accents, ect (ne fonctionne pas sur codeblock)
    SetConsoleOutputCP(CP_UTF8);

    // LAncer la classe game, afficher le nom et afficher le menu
    Game game = Game();
    game.start();
    game.showMenu();
}