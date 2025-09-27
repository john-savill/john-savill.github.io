/* Global Defines */

// Glicko system constants
#define INITIAL_RATING 1500.0
#define INITIAL_RD 200.0
#define INITIAL_VOLATILITY 0.06
#define TAU 0.5
#define CONVERGENCE_TOLERANCE 0.000001

// System limits
#define MAX_PLAYERS 1000
#define MAX_GAMES 10000
#define MAX_NAME_LENGTH 50

/* Global structures */

// Player structure
typedef struct {
    char name[MAX_NAME_LENGTH];
    double rating;
    double rating_deviation;
    double volatility;
    int games_played;
    double total_score_contribution; // track scoring performance
} Player;

// Game structure with exact scores
typedef struct {
    char team1_players[3][MAX_NAME_LENGTH];
    char team2_players[3][MAX_NAME_LENGTH];
    int team1_score; // Exact score (0-15)
    int team2_score;
    double score_margin; // Calculated margin factor
} Game;

/* Global variables */

extern Player players[MAX_PLAYERS];
extern Game games[MAX_GAMES];
extern int num_players;
extern int num_games;

/* Global functions prototypes */

extern int find_player_index(const char *name);
extern void add_player(const char *name);
extern void update_glicko_rating_with_scores(int player_index, double opponent_rating, 
                                    double opponent_RD, double outcome, double margin_factor);
extern void process_game(int game_index);
extern void read_csv_file(const char *filename);
extern void print_ratings();
extern double g_function(double RD);
extern double E_function(double rating, double opponent_rating, double opponent_RD);
extern double calculate_score_margin_factor(int score1, int score2);
extern double calculate_game_outcome(int team_score, int opponent_score);
extern void calculate_team_stats(char team_players[3][MAX_NAME_LENGTH], 
                         double *avg_rating, double *avg_RD);