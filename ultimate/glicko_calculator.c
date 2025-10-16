#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "glicko_calculator.h"

// Global variables
Player players[MAX_PLAYERS];
Game games[MAX_GAMES];
int num_players = 0;
int num_games = 0;

int main() {
    printf("Glicko Rating Calculator\n");
    printf("====================================\n\n");
    
    // Read game data from CSV file
    read_csv_file("scores.csv");
    
    printf("Processing %d games with %d players...\n\n", num_games, num_players);
    
    // Process all games
    int num_games_counter;
    for (num_games_counter = 0; num_games_counter < num_games; num_games_counter++) {
        process_game(num_games_counter);
    }
    
    // Display final ratings
    print_ratings();
    
    return 0;
}

// Find player by name
int find_player_index(const char *name) {
    int num_players_counter;
    for (num_players_counter = 0; num_players_counter < num_players; num_players_counter++) {
        if (strcmp(players[num_players_counter].name, name) == 0) {
            return num_players_counter;
        }
    }
    return -1;
}

// Add new player with default ratings
void add_player(const char *name) {
    if (num_players >= MAX_PLAYERS) {
        printf("Maximum number of players reached!\n");
        return;
    }
    
    strcpy(players[num_players].name, name);
    players[num_players].rating = INITIAL_RATING;
    players[num_players].rating_deviation = INITIAL_RD;
    players[num_players].volatility = INITIAL_VOLATILITY;
    players[num_players].games_played = 0;
    players[num_players].total_score_contribution = 0.0;
    num_players++;
}

// Calculate score margin factor (0.5 = close, 1.0 = blowout)
double calculate_score_margin_factor(int score1, int score2) {
    if (score1 == 0 && score2 == 0) return 0.5; // Handle edge case
    
    int total_points = score1 + score2;
    double margin = abs(score1 - score2);
    
    // Normalize margin influence
    double margin_factor = 0.5 + (margin / (total_points * 2.0)) * 0.5;
    return fmin(margin_factor, 1.0);
}

// Outcome calculation based on exact scores
double calculate_game_outcome(int team_score, int opponent_score) {
    if (team_score > opponent_score) {
        // Win with strength based on margin (0.75-1.0 range)
        double margin_strength = (double)(team_score - opponent_score) / 15.0;
        return 0.75 + (margin_strength * 0.25);
    } else if (team_score < opponent_score) {
        // Loss with severity based on margin (0.0-0.25 range)
        double margin_strength = (double)(opponent_score - team_score) / 15.0;
        return fmax(0.25 - (margin_strength * 0.25), 0.0);
    } else {
        return 0.5; // Draw
    }
}

// Calculate team average rating and RD
void calculate_team_stats(char team_players[3][MAX_NAME_LENGTH], 
                         double *avg_rating, double *avg_RD) {
    double total_rating = 0.0;
    double total_RD = 0.0;
    int valid_players = 0;
    
    int find_player_counter;
    for (find_player_counter = 0; find_player_counter < 3; find_player_counter++) {
        int player_idx = find_player_index(team_players[find_player_counter]);
        if (player_idx != -1) {
            total_rating += players[player_idx].rating;
            total_RD += players[player_idx].rating_deviation;
            valid_players++;
        }
    }
    
    *avg_rating = (valid_players > 0) ? total_rating / valid_players : INITIAL_RATING;
    *avg_RD = (valid_players > 0) ? total_RD / valid_players : INITIAL_RD;
}

// Glicko rating update with score consideration
void update_glicko_rating_with_scores(int player_index, double opponent_rating, 
                                    double opponent_RD, double outcome, double margin_factor) {
    Player *player = &players[player_index];
    
    // Glicko algorithm calculations
    double g_val = g_function(opponent_RD);
    double E_val = E_function(player->rating, opponent_rating, opponent_RD);
    
    // Apply margin factor to enhance rating change magnitude
    double scored_outcome = outcome;
    double rating_change_factor = margin_factor;
    
    // Calculate d² (measure of uncertainty)
    double d_squared = 1.0 / (g_val * g_val * E_val * (1.0 - E_val));
    
    // Calculate rating change (with margin factor)
    double rating_change = (g_val / (1.0 / (player->rating_deviation * player->rating_deviation) + 1.0 / d_squared)) 
                          * (scored_outcome - E_val) * rating_change_factor;
    
    // Update rating
    player->rating += rating_change;
    
    // Update rating deviation
    double new_RD_sq = 1.0 / (1.0 / (player->rating_deviation * player->rating_deviation) + 1.0 / d_squared);
    player->rating_deviation = sqrt(new_RD_sq);
    
    // Update volatility (simplified)
    player->volatility = fmax(player->volatility * 0.99, 0.01);
    
    player->games_played++;
}

// Process game with exact score consideration
void process_game(int game_index) {
    Game *game = &games[game_index];
    
    // Calculate score margin factor
    game->score_margin = calculate_score_margin_factor(game->team1_score, game->team2_score);
    
    // Calculate team statistics
    double team1_avg_rating, team1_avg_RD;
    double team2_avg_rating, team2_avg_RD;
    
    calculate_team_stats(game->team1_players, &team1_avg_rating, &team1_avg_RD);
    calculate_team_stats(game->team2_players, &team2_avg_rating, &team2_avg_RD);
    
    // Process Team 1 players
    double team1_outcome = calculate_game_outcome(game->team1_score, game->team2_score);

    int process_player_T1_counter;
    for (process_player_T1_counter = 0; process_player_T1_counter < 3; process_player_T1_counter++) {
        int player_idx = find_player_index(game->team1_players[process_player_T1_counter]);
        if (player_idx != -1) {
            // Track scoring contribution
            players[player_idx].total_score_contribution += (double)game->team1_score / 3.0;
            
            // Update rating with algorithm
            update_glicko_rating_with_scores(player_idx, team2_avg_rating, 
                                           team2_avg_RD, team1_outcome, game->score_margin);
        }
    }
    
    // Process Team 2 players
    double team2_outcome = calculate_game_outcome(game->team2_score, game->team1_score);

    int process_player_T2_counter;
    for (process_player_T2_counter = 0; process_player_T2_counter < 3; process_player_T2_counter++) {
        int player_idx = find_player_index(game->team2_players[process_player_T2_counter]);
        if (player_idx != -1) {
            // Track scoring contribution
            players[player_idx].total_score_contribution += (double)game->team2_score / 3.0;
            
            // Update rating with algorithm
            update_glicko_rating_with_scores(player_idx, team1_avg_rating, 
                                           team1_avg_RD, team2_outcome, game->score_margin);
        }
    }
    
    // Debug output for significant games
    if (abs(game->team1_score - game->team2_score) >= 10) {
        printf("Blowout Game: %d-%d (Margin Factor: %.2f)\n", 
               game->team1_score, game->team2_score, game->score_margin);
    }
}

// CSV reader supporting exact scores
void read_csv_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }
    
    char line[500];
    
    // Skip header line
    if (fgets(line, sizeof(line), file) == NULL) {
        printf("Error: Empty file\n");
        fclose(file);
        return;
    }
    
    while (fgets(line, sizeof(line), file) && num_games < MAX_GAMES) {
        // Parse CSV: team1_p1,team1_p2,team1_p3,team2_p1,team2_p2,team2_p3,team1_score,team2_score
        char *token = strtok(line, ",");
        int field = 0;
        
        // Parse team 1 players
        int parse_T1_counter;
        for (parse_T1_counter = 0; parse_T1_counter < 3 && token; parse_T1_counter++) {
            strcpy(games[num_games].team1_players[parse_T1_counter], token);
            // Add player if not exists
            if (find_player_index(token) == -1) {
                add_player(token);
            }
            token = strtok(NULL, ",");
        }
        
        // Parse team 2 players
        int parse_T2_counter;
        for (parse_T2_counter = 0; parse_T2_counter < 3 && token; parse_T2_counter++) {
            strcpy(games[num_games].team2_players[parse_T2_counter], token);
            // Add player if not exists
            if (find_player_index(token) == -1) {
                add_player(token);
            }
            token = strtok(NULL, ",");
        }
        
        // Parse exact scores
        if (token) {
            games[num_games].team1_score = atoi(token);
            token = strtok(NULL, ",\n\r");
        }
        if (token) {
            games[num_games].team2_score = atoi(token);
        }
        
        // Validate scores
        if (games[num_games].team1_score < 0 || games[num_games].team1_score > 15 ||
            games[num_games].team2_score < 0 || games[num_games].team2_score > 15) {
            printf(" Warning: Invalid scores in game %d (%d-%d)\n", 
                   num_games + 1, games[num_games].team1_score, games[num_games].team2_score);
        }
        
        num_games++;
    }
    
    fclose(file);
    printf("Successfully loaded %d games from %s\n\n", num_games, filename);
}

// Rating display with scoring stats
void print_ratings() {
    printf("\nFINAL PLAYER RATINGS (Glicko System)\n");
    printf("================================================\n");
    printf("%-20s %8s %8s %10s %6s %8s\n", 
           "|Player|", "|Rating|", "|RD|", "|Volatility|", "|Games|", "|Avg. score contribution per game|");
    printf("------------------------------------------------\n");
    
    // Sort players by rating
    int player_sort_counter;
    for (player_sort_counter = 0; player_sort_counter < num_players - 1; player_sort_counter++) {
        int rating_sort_counter;
        for (rating_sort_counter = player_sort_counter + 1; rating_sort_counter < num_players; rating_sort_counter++) {
            if (players[player_sort_counter].rating < players[rating_sort_counter].rating) {
                Player temp = players[player_sort_counter];
                players[player_sort_counter] = players[rating_sort_counter];
                players[rating_sort_counter] = temp;
            }
        }
    }
    
    // Display sorted ratings
    int display_counter;
    for (display_counter = 0; display_counter < num_players; display_counter++) {
        double avg_score = (players[display_counter].games_played > 0) ? 
                          players[display_counter].total_score_contribution / players[display_counter].games_played : 0.0;
        
        printf("%-20s %8.0f %8.1f %10.3f %6d %8.1f\n",
               players[display_counter].name,
               players[display_counter].rating,
               players[display_counter].rating_deviation,
               players[display_counter].volatility,
               players[display_counter].games_played,
               avg_score);
    }
}

// Glicko helper functions
double g_function(double RD) {
    return 1.0 / sqrt(1.0 + 3.0 * RD * RD / (M_PI * M_PI));
}

double E_function(double rating, double opponent_rating, double opponent_RD) {
    return 1.0 / (1.0 + exp(-g_function(opponent_RD) * (rating - opponent_rating) / 400.0));
}