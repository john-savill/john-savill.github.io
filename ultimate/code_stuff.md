# Some thoughts

Listening to how Jake Smart used an algorithmic approach to determine the best handler dominator setup from the Brown University team of 2020's sounded interesting. 

I have had some experience using rating algorithms, uisng Glicko(V2) on a little side project to rate **very** amateur tennis players and predict matches, just a fun thing for some friends. 

I work as a software engineer so i'd have a go at creating something similar to Jake Smart's application, using the glicko alorithm.

## Glicko

I won't go into detail about the Glicko algorithm itself, it's well [documented](https://www.glicko.net/glicko.html) and, if you're that curious, any reasearch you will do will be much more useful than my explanation.

Initial ratings are needed before any of the game/score analysis is done, this default is usually 1500.

After each game algorithm analyzes the outcome and updates the rating of the players involved. Also updated is the rating deviation (RD), which is a measure of confidence in the rating, depending on the number of games and recency. In this application case we calculate team average ratings when processing scores. If a player has a higer RD their rating will change more dramatically after each game processed.

The volatility of each player represents the degree of expected fluctuation. The volatility is low if the player performs consistently but will be higher if the player has eratic results.

The ratings for players are updated after every game.

A small note on the adavantages over standard ELO:
 - You get more accurate ratings with the inclusion of RD. It provides a more accurate representation of a player's skill, especially for players with limited game history or who haven't played recently. 
 - Glicko handles Inactivity. The RD helps to account for inactivity, meaning a player's rating won't be as drastically affected by a period of inactivity compared to Elo. 
 - Glicko's system is generally better at predicting game outcomes than Elo, but that is somehting that isn't added, yet...

## Code explanation

Again, I won't go into too much detail, and will just give an overview. There's a few comments dotted about the code and C is fairly understandable.

The definitions of system limits and initial variables are defined in the header file. This is probably the only thing you'd want to change.

The structures, variables and functions are also included here, in case you were wanted to add to the logic in any way and call the individual functions from another file. The player struct stores player name, rating, rating deviation (RD), volatility, and games played, the Game struct represents a game between two teams of 3 players, each with scores.

Each function is described in the table below

| Function | Description |
| - | - |
| main() | Overall reads data from CSV, processes games, prints ratings |
| find_player_index() | Searches for a player by name |
| add_player() | Creates a new player with default ratings |
| update_glicko_rating() | Core algorithm for updating player ratings |
| process_game() | Processes game results and updates all players' ratings |
| read_csv_file() | Parses game data from CSV files |
| print_ratings() | Outputs sorted player ratings |
| g_function() | Helper functions for the Glicko algorithm |
| E_function() | Helper functions for the Glicko algorithm |
| calculate_score_margin_factor() | Simply handles if the game was close or a blowout |
| calculate_game_outcome() | Win/lose calculation based on exact scores  |
| calculate_team_stats() | Calculate team average rating and RD, for per-game processing |

The process_game() function specifically:
 - Calculates average team ratings and rating deviations
 - Determines game outcome (win/loss/draw)
 - Updates each player's rating based on team performance
 - Uses the Glicko algorithm to calculate rating changes

Obviously we are handling 3 players competing on each team with each player's rating updated based on the outcome of the game and the strength of the opposing team.

Running the final executable should return somehting similar to the following (based off the example scores.csv)

```
Glicko Rating Calculator
====================================

✅ Successfully loaded 5 games from scores.csv

Processing 5 games with 8 players...


🏆 FINAL PLAYER RATINGS (Glicko System)
================================================
|Player|             |Rating|     |RD| |Volatility| |Games| |Contribution Score per game|
------------------------------------------------
blvaro                   1578     54.6      0.057      5      1.5
clvaro                   1555     61.1      0.057      5      1.3
glvaro                   1539     69.1      0.059      2      1.7
alvaro                   1498     69.4      0.058      4      0.8
dlvaro                   1474     56.2      0.058      4      1.0
hlvaro                   1473     72.5      0.059      1      0.7
flvaro                   1458     60.2      0.058      4      0.8
elvaro                   1419     51.8      0.057      5      0.7
```
## Limitations

I have added the scores.csv used for testing. Obviously the code expects the scores for each 3v3 game to be in the same format, but it is a .csv file it is reading so it easy enough to edit and add rows for each set of games/scores.

There are system limits defined in the header but these can be changed.

We are limited to 3v3 games with no accountability for specific game time. So although number of games is taken into account the actual time between games isn't.

Ultimate speicific parts of a games score, i.e breaks, are not used in calculating players rating. Who started on offence isn't recorded either, which can make a difference in low-scoring 3v3 games. It's just a cold, rating calculating algorithm that doesn't represent actual player skill.

## Building/ Using

Trustworthy people can just download the C code and header files and build it using a compiler themselves 
```
gcc -o glicko_calculator glicko_calculator.c -lm
```
Then just run the executable with the "scores.csv" in the same directory to run the analysis and return the ratings.
```
./glicko_calculator
```
### And if you're interested...

I have been doing a fair bit of CI/CD and have created a .yml for builing, testing, and deploying the code, whilst describing the process. This isn't relevant to the actual glicko ranking application itself, just making it be system agnostic.

The .yml will build the code, validate it, then containerise it. I've addewd a python script that also runs and uses the .yml to create a plantUML activity diagram. 

This just means that updates to the code can be tested automatically and any enhancements/ other functions (like game prediction, front-end input api, accounting for other metrics) can be added and tested automatically.

At the moment the .yml just tests the functionality of the file, not the algorithm itself.


