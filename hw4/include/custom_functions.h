#define WRONG_ARG_COUNT(length, required) printf("Wrong number of args (given: %d, required: %d) for CLI command 'help'\n", length, required); sf_cmd_error("arg count");
#define CHECK_ARG(length, required) if (length != required) {WRONG_ARG_COUNT(length, required); goto bad_arg;}

/*
 * Structure that describes a printer.
 */
typedef struct printer {
    char *name;
    FILE_TYPE *type;
    PRINTER_STATUS status;
} PRINTER;

/*
 * Space to store the PRINTERs.
 */
PRINTER printers[MAX_PRINTERS];
// Global counter to keep track of the number of printers
int printer_count;

/*
 * Structure that describes a job.
 * You must define this appropriately (but not here).
 */
typedef struct job {
    char *file;
    int eligible; // 32 bits. One for each printer (1 if eligible)
    FILE_TYPE *type;
    JOB_STATUS status;
} JOB;

/*
 * Space to store the JOBs.
 */
JOB jobs[MAX_JOBS];
// Global counter to keep track of the number of jobs. (64 bits - if bit is 1 then it is taken)
size_t job_count;

/*
 * Space to store the PRINTER/JOB group pids.
 */
pid_t printer_pids[MAX_PRINTERS];
pid_t job_pids[MAX_JOBS];
// Counter for number jobs being processed
int job_process_count;
// Flag to keep track of when things are done.
volatile sig_atomic_t jobs_done;


/**
 * Splits a string using whitespaces as the delimiter. The 
 * strings in the array do not have whitespaces.
 * 
 * @param string The string to split
 * @param length The int pointer to update to the length 
 * 
 * @return NULL if the string is empty else an array of the strings 
 * split by whitespaces.
 */
char **split_string(char *string, int *length);

/**
 * Defines a new printer, with a given name and file type.
 * 
 * @param name The name of the printer 
 * @param type A pointer to a FILE_TYPE of the file type
 * 
 * @return a newly created printer object with the sepcific name for 
 * the given file type or NULL, if MAX_PRINTERS have been defined.
 */
PRINTER *define_printer(char *name, FILE_TYPE *type);

/**
 * Lookup a new printer by its name.
 * 
 * @param name The name of the printer 
 * 
 * @return the PRINTER object with the specific name, or NULL, 
 * if no printer with this name has been defined. 
 */
PRINTER *find_printer_name(char *name);

/**
 * Define a new job for the given file, with the given eligible printers.
 *
 * @param file The name of the file to print
 * @param type A pointer to a FILE_TYPE of the file type
 * @param printer_set An integer tracking the set of eligible printers
 * 
 * @return the JOB object for the given file, or NULL, if MAX_JOBS
 * have been defined.
 */
JOB *create_job(char *file, FILE_TYPE *type, int printer_set);

/**
 * Process a job using the provided printer.
 */
pid_t start_job(PRINTER *printer, JOB *job);

/**
 * Pause the job specified by the id number [0, MAX_JOBS).
 */
void pause_job(int id);

/**
 * Resume the job specified by the id number [0, MAX_JOBS).
 */
void resume_job(int id);

/**
 * Cancel the job specified by the id number [0, MAX_JOBS).
 */
void cancel_job(int id);

/**
 * Delete the job specified by the id number [0, MAX_JOBS) 
 * for the jobs array.
 */
void delete_job(int id);

/**
 * Scans jobs array for each printer.
 */ 
void scanner();


/**
 * SIGCHLD handler that monitors the jobs.
 */
void job_handler(int sig);