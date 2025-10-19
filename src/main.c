#include "rforth.h"

/* Function prototypes */
static void print_usage(const char *program_name);
static void print_version(void);

int main(int argc, char *argv[]) {
    bool repl_mode = false;
    bool compile_mode = false;
    char *input_file = NULL;
    char *output_file = NULL;
    
    /* Parse command line options - Windows style */
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        
        /* Check for option flags (- or /) */
        if (arg[0] == '-' || arg[0] == '/') {
            char flag = arg[1];
            
            switch (flag) {
                case 'h':
                case '?':
                    print_usage(argv[0]);
                    return 0;
                case 'v':
                    print_version();
                    return 0;
                case 'r':
                    repl_mode = true;
                    break;
                case 'c':
                    compile_mode = true;
                    break;
                case 'i':
                    /* Next argument is input file */
                    if (i + 1 < argc) {
                        input_file = argv[++i];
                    } else {
                        fprintf(stderr, "Error: -i requires a filename\n");
                        print_usage(argv[0]);
                        return 1;
                    }
                    break;
                case 'o':
                    /* Next argument is output file */
                    if (i + 1 < argc) {
                        output_file = argv[++i];
                    } else {
                        fprintf(stderr, "Error: -o requires a filename\n");
                        print_usage(argv[0]);
                        return 1;
                    }
                    break;
                default:
                    fprintf(stderr, "Error: Unknown option -%c\n", flag);
                    print_usage(argv[0]);
                    return 1;
            }
        } else {
            /* Non-option argument - treat as input file if not already set */
            if (!input_file) {
                input_file = arg;
            }
        }
    }
    
    /* Initialize I/O system */
    io_ctx_t *io_ctx = io_init();
    if (!io_ctx) {
        fprintf(stderr, "Error: Failed to initialize I/O system\n");
        return 1;
    }
    
    /* Initialize RForth context */
    rforth_ctx_t *ctx = rforth_init();
    if (!ctx) {
        io_error_string("Error: Failed to initialize RForth\n");
        io_cleanup(io_ctx);
        return 1;
    }
    
    int result = 0;
    
    /* Determine mode and execute */
    if (compile_mode) {
        if (!input_file) {
            fprintf(stderr, "Error: Input file required for compile mode\n");
            print_usage(argv[0]);
            result = 1;
        } else if (!output_file) {
            fprintf(stderr, "Error: Output file required for compile mode (-o)\n");
            print_usage(argv[0]);
            result = 1;
        } else {
            printf("Compiling %s to %s...\n", input_file, output_file);
            result = rforth_compile_file(ctx, input_file, output_file);
            if (result == 0) {
                printf("Compilation successful.\n");
            } else {
                fprintf(stderr, "Compilation failed.\n");
            }
        }
    } else if (repl_mode) {
        printf("RForth v%d.%d.%d Interactive\n", 
               RFORTH_VERSION_MAJOR, RFORTH_VERSION_MINOR, RFORTH_VERSION_PATCH);
        printf("Type 'bye' to exit.\n\n");
        result = rforth_repl(ctx);
    } else if (input_file) {
        /* Interpret file */
        result = rforth_interpret_file(ctx, input_file);
    } else {
        /* No mode specified, default to REPL */
        printf("RForth v%d.%d.%d Interactive\n", 
               RFORTH_VERSION_MAJOR, RFORTH_VERSION_MINOR, RFORTH_VERSION_PATCH);
        printf("Type 'bye' to exit.\n\n");
        result = rforth_repl(ctx);
    }
    
    /* Cleanup */
    rforth_cleanup(ctx);
    io_cleanup(io_ctx);
    return result;
}

static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] [FILE]\n", program_name);
    printf("\nOptions:\n");
    printf("  -h          Show this help message\n");
    printf("  -v          Show version information\n");
    printf("  -r          Start REPL mode\n");
    printf("  -c          Compile mode (requires -o)\n");
    printf("  -i FILE     Interpret FILE\n");
    printf("  -o FILE     Output file for compile mode\n");
    printf("\nExamples:\n");
    printf("  %s -r                    # Start REPL\n", program_name);
    printf("  %s hello.f               # Interpret hello.f\n", program_name);
    printf("  %s -i hello.f            # Interpret hello.f\n", program_name);
    printf("  %s -c hello.f -o hello   # Compile hello.f to executable\n", program_name);
}

static void print_version(void) {
    printf("RForth v%d.%d.%d\n", 
           RFORTH_VERSION_MAJOR, RFORTH_VERSION_MINOR, RFORTH_VERSION_PATCH);
    printf("A Forth interpreter and compiler\n");
}