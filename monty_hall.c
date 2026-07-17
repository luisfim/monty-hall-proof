/*
 * Monty Hall Simulator
 *
 * A terminal-based Monte Carlo simulation comparing two strategies:
 * always staying with the first door and always switching doors.
 *
 * Build:
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -O2 monty_hall.c -o monty_hall
 */

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_TRIALS UINT64_C(100000000)
#define BAR_WIDTH 40

#define ANSI_RESET   "\x1b[0m"
#define ANSI_BOLD    "\x1b[1m"
#define ANSI_DIM     "\x1b[2m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_MAGENTA "\x1b[35m"

typedef struct {
    uint64_t wins;
    uint64_t losses;
} StrategyResult;

typedef struct {
    StrategyResult stay;
    StrategyResult switch_doors;
    uint64_t initially_correct;
    uint64_t initially_wrong;
} SimulationResult;

static uint64_t rng_state;
static int use_color = 1;

static const char *color(const char *code)
{
    return use_color ? code : "";
}

#ifdef _WIN32
static int enable_windows_ansi(void)
{
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    if (output == INVALID_HANDLE_VALUE || !GetConsoleMode(output, &mode)) {
        return 0;
    }

    return SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
}
#endif

static void configure_terminal(void)
{
    if (getenv("NO_COLOR") != NULL) {
        use_color = 0;
        return;
    }

#ifdef _WIN32
    use_color = enable_windows_ansi();
#endif
}

/*
 * SplitMix64 is small, fast, and more than sufficient for this educational
 * simulation. It is not intended for cryptographic use.
 */
static uint64_t random_u64(void)
{
    uint64_t value;

    rng_state += UINT64_C(0x9E3779B97F4A7C15);
    value = rng_state;
    value = (value ^ (value >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94D049BB133111EB);
    return value ^ (value >> 31);
}

static void seed_random_generator(void)
{
    uint64_t seed = (uint64_t)time(NULL);
    seed ^= (uint64_t)clock() << 32;
    seed ^= (uint64_t)(uintptr_t)&seed;

    rng_state = seed;
    (void)random_u64();
}

/* Returns an unbiased random integer in the range [0, upper_bound). */
static uint64_t random_below(uint64_t upper_bound)
{
    const uint64_t threshold = (uint64_t)(-upper_bound) % upper_bound;
    uint64_t value;

    do {
        value = random_u64();
    } while (value < threshold);

    return value % upper_bound;
}

static int random_door(void)
{
    return (int)random_below(3);
}

/*
 * Monty must open a door that:
 *   1. was not selected by the player; and
 *   2. does not contain the prize.
 *
 * If Monty has two valid goat doors, he chooses one at random.
 */
static int choose_host_door(int prize_door, int first_choice)
{
    int valid_doors[2];
    int count = 0;
    int door;

    for (door = 0; door < 3; ++door) {
        if (door != prize_door && door != first_choice) {
            valid_doors[count++] = door;
        }
    }

    if (count == 1) {
        return valid_doors[0];
    }

    return valid_doors[random_below(2)];
}

static int remaining_closed_door(int first_choice, int host_door)
{
    /*
     * Door indexes are 0, 1, and 2, whose sum is 3.
     * Subtracting the two known doors leaves the final closed door.
     */
    return 3 - first_choice - host_door;
}

static SimulationResult run_simulation(uint64_t trials)
{
    SimulationResult results = {0};
    uint64_t trial;

    for (trial = 0; trial < trials; ++trial) {
        const int prize_door = random_door();
        const int first_choice = random_door();
        const int host_door = choose_host_door(prize_door, first_choice);
        const int switched_choice =
            remaining_closed_door(first_choice, host_door);

        if (first_choice == prize_door) {
            ++results.initially_correct;
            ++results.stay.wins;
            ++results.switch_doors.losses;
        } else {
            ++results.initially_wrong;
            ++results.stay.losses;
            ++results.switch_doors.wins;
        }

        /*
         * This comparison explicitly uses the door left after Monty opens a
         * goat door. Under the standard rules, it must match the logic above.
         */
        if ((switched_choice == prize_door) !=
            (first_choice != prize_door)) {
            fputs("Internal simulation error.\n", stderr);
            exit(EXIT_FAILURE);
        }
    }

    return results;
}

static double percentage(uint64_t amount, uint64_t total)
{
    return total == 0 ? 0.0 : ((double)amount * 100.0) / (double)total;
}

static void print_integer(uint64_t value)
{
    char digits[32];
    char formatted[48];
    size_t length;
    size_t source = 0;
    size_t destination = 0;
    size_t first_group;

    (void)snprintf(digits, sizeof(digits), "%" PRIu64, value);
    length = strlen(digits);
    first_group = length % 3;

    if (first_group == 0) {
        first_group = 3;
    }

    while (source < length) {
        formatted[destination++] = digits[source++];

        if (source == first_group && source < length) {
            formatted[destination++] = ',';
        } else if (source > first_group &&
                   (source - first_group) % 3 == 0 &&
                   source < length) {
            formatted[destination++] = ',';
        }
    }

    formatted[destination] = '\0';
    fputs(formatted, stdout);
}

static void print_bar(double rate)
{
    int filled = (int)((rate / 100.0) * BAR_WIDTH + 0.5);
    int position;

    if (filled < 0) {
        filled = 0;
    } else if (filled > BAR_WIDTH) {
        filled = BAR_WIDTH;
    }

    putchar('[');
    for (position = 0; position < BAR_WIDTH; ++position) {
        putchar(position < filled ? '#' : '.');
    }
    printf("] %6.2f%%", rate);
}

static void print_strategy(const char *name,
                           const char *name_color,
                           StrategyResult result,
                           uint64_t trials)
{
    const double win_rate = percentage(result.wins, trials);
    const double loss_rate = percentage(result.losses, trials);

    printf("\n  %s%s%s\n",
           color(name_color), name, color(ANSI_RESET));

    printf("  Wins   : ");
    print_integer(result.wins);
    printf("  (%6.2f%%)\n", win_rate);

    printf("  Losses : ");
    print_integer(result.losses);
    printf("  (%6.2f%%)\n", loss_rate);

    printf("  ");
    print_bar(win_rate);
    putchar('\n');
}

static void print_header(void)
{
    printf("%s%s", color(ANSI_BOLD), color(ANSI_CYAN));
    puts("+====================================================================+");
    puts("|                     MONTY HALL SIMULATOR                           |");
    puts("|              Stay with your door, or switch?                       |");
    puts("+====================================================================+");
    printf("%s", color(ANSI_RESET));
}

static void print_introduction(void)
{
    puts("\nThere are three doors: one hides a prize and two hide goats. A player");
    puts("chooses a door at random. The host knows where the prize is, opens a");
    puts("different door containing a goat, and offers a switch. This program");
    puts("runs the same random games for both strategies to compare what happens");
    puts("when the player always stays and when the player always switches.\n");
}

static int parse_trials(const char *input, uint64_t *trials)
{
    char cleaned[128];
    char *end;
    size_t source = 0;
    size_t destination = 0;
    unsigned long long parsed;

    while (input[source] != '\0' && destination + 1 < sizeof(cleaned)) {
        const unsigned char character = (unsigned char)input[source++];

        if (isdigit(character)) {
            cleaned[destination++] = (char)character;
        } else if (character == ',' || character == '_' ||
                   isspace(character)) {
            continue;
        } else {
            return 0;
        }
    }

    if (destination == 0 || input[source] != '\0') {
        return 0;
    }

    cleaned[destination] = '\0';
    errno = 0;
    parsed = strtoull(cleaned, &end, 10);

    if (errno == ERANGE || *end != '\0' ||
        parsed == 0 || parsed > MAX_TRIALS) {
        return 0;
    }

    *trials = (uint64_t)parsed;
    return 1;
}

static uint64_t ask_for_trials(void)
{
    char input[128];
    uint64_t trials;

    for (;;) {
        printf("%sHow many games should each strategy play?%s\n",
               color(ANSI_BOLD), color(ANSI_RESET));
        printf("%sEnter a number from 1 to 100,000,000: %s",
               color(ANSI_DIM), color(ANSI_RESET));

        if (fgets(input, sizeof(input), stdin) == NULL) {
            puts("\nNo input received.");
            exit(EXIT_FAILURE);
        }

        if (strchr(input, '\n') == NULL && !feof(stdin)) {
            int character;
            while ((character = getchar()) != '\n' && character != EOF) {
                /* Discard the rest of an overly long input line. */
            }
            puts("Please enter a shorter valid number.\n");
            continue;
        }

        if (parse_trials(input, &trials)) {
            return trials;
        }

        printf("%sInvalid value.%s Use digits only, optionally with commas "
               "or underscores.\n\n",
               color(ANSI_RED), color(ANSI_RESET));
    }
}

static void print_results(const SimulationResult *results,
                          uint64_t trials,
                          double elapsed_seconds)
{
    const double correct_rate =
        percentage(results->initially_correct, trials);
    const double wrong_rate =
        percentage(results->initially_wrong, trials);
    const double stay_rate =
        percentage(results->stay.wins, trials);
    const double switch_rate =
        percentage(results->switch_doors.wins, trials);
    const double point_difference = switch_rate - stay_rate;

    printf("\n%s%s+--------------------------- RESULTS -------------------------------+%s\n",
           color(ANSI_BOLD), color(ANSI_MAGENTA), color(ANSI_RESET));

    printf("\n  Random games evaluated per strategy : ");
    print_integer(trials);
    printf("\n  Total strategy outcomes calculated   : ");
    print_integer(trials * 2);
    printf("\n  Simulation time                      : %.3f seconds\n",
           elapsed_seconds);

    printf("\n%sINITIAL RANDOM CHOICES%s\n",
           color(ANSI_BOLD), color(ANSI_RESET));
    printf("  Prize chosen immediately : ");
    print_integer(results->initially_correct);
    printf("  (%6.2f%%)\n", correct_rate);
    printf("  Goat chosen immediately  : ");
    print_integer(results->initially_wrong);
    printf("  (%6.2f%%)\n", wrong_rate);

    printf("\n%sSTRATEGY RESULTS%s\n",
           color(ANSI_BOLD), color(ANSI_RESET));
    print_strategy("STAY WITH THE FIRST DOOR",
                   ANSI_YELLOW,
                   results->stay,
                   trials);
    print_strategy("SWITCH TO THE OTHER DOOR",
                   ANSI_GREEN,
                   results->switch_doors,
                   trials);

    printf("\n%sCOMPARISON%s\n",
           color(ANSI_BOLD), color(ANSI_RESET));
    printf("  Switching advantage : %+.2f percentage points\n",
           point_difference);

    if (stay_rate > 0.0) {
        printf("  Win-rate ratio       : %.2fx as many wins by switching\n",
               switch_rate / stay_rate);
    } else {
        puts("  Win-rate ratio       : unavailable (staying had zero wins)");
    }

    puts("  Theoretical stay     : 33.33%");
    puts("  Theoretical switch   : 66.67%");

    printf("\n%s%sCONCLUSION%s\n",
           color(ANSI_BOLD), color(ANSI_CYAN), color(ANSI_RESET));

    if (switch_rate > stay_rate) {
        printf("  These trials favored %sswitching%s. With a large sample, the rates\n",
               color(ANSI_GREEN), color(ANSI_RESET));
    } else if (switch_rate < stay_rate) {
        printf("  This small sample happened to favor staying, but larger samples\n");
    } else {
        printf("  This sample ended in a tie, but larger samples\n");
    }

    puts("  should move toward about 1/3 for staying and 2/3 for switching.");
    puts("  Under the standard Monty Hall rules, switching doubles the chance");
    puts("  of winning because it wins whenever the first random choice was wrong.");

    printf("\n%sTip:%s Try 10,000, 1,000,000, or more games and compare the rates.\n",
           color(ANSI_BOLD), color(ANSI_RESET));
    printf("%s+====================================================================+%s\n",
           color(ANSI_CYAN), color(ANSI_RESET));
}

int main(void)
{
    uint64_t trials;
    SimulationResult results;
    clock_t start;
    clock_t finish;
    double elapsed_seconds;

    configure_terminal();
    seed_random_generator();

    print_header();
    print_introduction();
    trials = ask_for_trials();

    printf("\n%sRunning simulation...%s\n",
           color(ANSI_CYAN), color(ANSI_RESET));

    start = clock();
    results = run_simulation(trials);
    finish = clock();

    elapsed_seconds = (double)(finish - start) / (double)CLOCKS_PER_SEC;
    print_results(&results, trials, elapsed_seconds);

    return EXIT_SUCCESS;
}
