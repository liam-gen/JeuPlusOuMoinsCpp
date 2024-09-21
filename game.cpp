using namespace std;

#include <iostream>
#include <random>
#include <string>
#include <windows.h>
#include <chrono>
#include <map>

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

struct Setting {
    string id;
    string name;
    int value;
};

class Settings {

    public:
        vector<Setting> settings;

        Settings() {
            settings.push_back({ "min", "Nombre minimum", 0 });
            settings.push_back({ "max", "Nombre maximum", 100 });
        }

        int get(const string& id) {
            for (const auto& setting : settings) {
                if (setting.id == id) {
                    return setting.value;
                }
            }
            return -1;
        }

        string getName(const string& id) {
            for (const auto& setting : settings) {
                if (setting.id == id) {
                    return setting.name;
                }
            }
        }

        void setValue(const string& id, int valeur) {
            for (auto& setting : settings) {
                if (setting.id == id) {
                    setting.value = valeur;
                    return;
                }
            }
        }
};

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
       this->show("Entrez le nombre de joueurs : ", false);
       int nbPlayers = 2;
       cin >> nbPlayers;

       // Si il y a un problème (pas int ou pas sup à 2) on ignore l'entrée
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

        if (input == this->number)
        {
            player->addScore();

            this->show(this->style.get("Bravo !", "green"));
            this->ended = true;

            auto endTime = chrono::high_resolution_clock::now();

            chrono::duration<double> duration = endTime - this->startTime;
            this->show("Durée de la partie : "+to_string(duration.count())+"s");

            GameSession storeGame = GameSession();
            storeGame.duration = duration.count();
            storeGame.number = this->number;
            storeGame.score = player->score;

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

    int askForNumberCin()
    {
        int x;
        cin >> x;
        
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(1000, '\n');
            // TOFIX : Afficher les nombres des paramètres
            this->show("Vous devez entrer un nombre entre 0 et 100 !\nRéssayez : ", false);
            return this->askForNumberCin();
        }

        return x;
    }

    void askForNumber(Player* player)
    {
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
        this->show(this->style.get(this->style.get("Menu", "bold"), "underline"));
        this->show("======================");
        this->show("1. Afficher les règles du jeu");
        this->show("2. Afficher les parties précédentes");
        this->show("3. Lancer le jeu en solo");
        this->show("4. Lancer le jeu à plusieurs");
        this->show("5. Paramètres");

        this->show("\nEntrez une action : ", false);

        int x;
        cin >> x;

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
        };
    }

    void startSettings() {
        this->clear();
        this->start();

        this->show(this->style.get("Paramètres du jeu", "bold"));
        this->show("======================");
        for (const auto& setting : this->settings.settings) {
            this->show(this->style.get(setting.name, "bold") + " : " + to_string(setting.value));
        }
        this->show("\n\n");

        this->showSettingsMenu();
    }

    void showSettingsMenu()
    {
        this->show(this->style.get(this->style.get("Menu", "bold"), "underline"));
        this->show("======================");
        this->show("1. Retourner au menu");

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
            this->askForChangeSetting(x - 2);
        }

        
    }

    void askForChangeSetting(int index) {
        if (index < 0 || index >= this->settings.settings.size()) {
            cout << "Erreur: Index inconnu " << index << endl;
            return;
        }

        Setting& setting = this->settings.settings[index];

        cout << "\nChanger la valeur de \"" << setting.name << "\" : ";

        int x;
        cin >> x;

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
        string messages[] = { "Préparation du jeu", "Génération d'un nombre aléatoire", "Chargement... Normalement ça marche sur ma machine"};
        const int nombreDeMessages = sizeof(messages) / sizeof(messages[0]);
        int indexMessage = 0;

        for (int z = 0; z < nombreDeMessages; z++)
        {
            
            string message = messages[indexMessage];
            for (int x = 0; x < 4; x++)
            {
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
        this->clear();
        this->start();

        this->show(this->style.get("Règles du jeu", "bold"));
        this->show("======================");
        this->show(this->style.get("Solo :", "bold"));
        this->show("Le jeu choisis un nombre entre 1 et 100 puis il te demande quel nombre tu veux entrer.\nTu entres un nombre et le jeu te dit si c'est + ou -.\nTu gagnes lorsque tu as trouvé le nombre !");
        this->show(this->style.get("Multijoueurs", "bold"));
        this->show("Tu choisis un nombre de joueurs (a partir de 2).\nVous entrez vos surnoms puis vous lancez la partie.\nLe jeu choisis un nombre entre 0 et 100.\nVous jouez chacun votre tours (le jeu affiche le joueur)\nLe premier à trouver le nombre à gagné !");

        this->show("\n\n");

        this->showMenu();
    }

    void previousGames() {
        this->clear();
        this->start();

        this->show(this->style.get("Parties récentes", "bold"));
        this->show("======================");

        if (this->games.size() == 0)
        {
            this->show("Aucune partie récente :/");
        }
        else {
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
                    this->show("Type : Solo");
                    this->show("Durée : " + to_string(this->games[i].duration) + "s");
                    this->show("Nombre : " + to_string(this->games[i].number));
                    this->show("Tentatives : " + to_string(this->games[i].players[0].score));
                }
                else {
                    this->show("Type : Multijoeurs");
                    this->show("Durée : " + to_string(this->games[i].duration) + "s");
                    this->show("Nombre : " + to_string(this->games[i].number));
                    this->show("Joueurs :");
                    for (int x = 0; x < this->games[i].players.size(); x++)
                    {
                        Player user = this->games[i].players[x];
                        this->show(user.username + " avec " + to_string(user.score)+ " tentatives");
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
        this->initGame();

        Player player = Player();
        this->players.emplace_back(player);

        while (!this->ended)
        {

            // TOFIX: Recommencer quand c'est pas un nombre
            this->askForNumber(&this->players[0]);
        }
    }

    void initPlayers(int playersNb = 2)
    {

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
        this->initGame(true);

        while (!this->ended)
        {
            for (int i = 0; i < this->players.size(); i++) {
                // TOFIX: Recommencer quand c'est pas un nombre
                this->askForNumber(&players[i]);
            }
        }
    }
};

int main()
{
    // Accepter tout ce qui est accents, ect
    SetConsoleOutputCP(CP_UTF8);

    Game game = Game();
    game.start();
    game.showMenu();
}