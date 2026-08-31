/* Copy multiple separate files in parallel, where each thread 
handles exactly one file, start to finish, independently of the others.

Command to run: ./mmcopier n source_dir destination_dir
mmcopier - file we are in
n - amount of threads to use
source_dir - directory where the files are located
destination_dir - directory where the files will be copied to

First edit was made by: Adam Dimkovski
*/

#include <cstdio>      // file I/O
#include <cstdlib>     // exit codes, malloc etc
#include <pthread.h>   // threading
#include <cstring>     // string building
#include <iostream>    // testing
#include <sys/stat.h>  // validating directories

// Thread Data Struct
struct ThreadData {
    char source_path[256];
    char dest_path[256];
};

// Thread creation function
void *thread_function(void *arg) {

    // Cast arg back to your struct type
    ThreadData *data = (ThreadData *)arg;

    // Buffer and Size variables for the copy loop
    char buffer[4096];
    size_t bytes_read;

    // Reads a source path file to a FILE object, this is done in binary mode.
    FILE *src = fopen(data->source_path, "rb");
    
    // If thread doesnt exist, return error message
    if (src == nullptr) {
        std::cerr << "Error: could not open source file: " << data->source_path << "\n";
        return nullptr;
    }

    // Writes a source path file from a FILE object into a destination path file
    FILE *dst = fopen(data->dest_path, "wb");

    // If thread doesnt exist, return error message
    if (dst == nullptr) {
        std::cerr << "Error: could not open destination file: " << data->dest_path << "\n";
        fclose(src);
        return nullptr;
    }

    // Copy loop to Copy a file from source to destination
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
       if (fwrite(buffer, 1, bytes_read, dst) < bytes_read) {
            std::cerr << "Error: fwrite value is INCORRECT\n";
            break;
        }
    }

    // Must be closed to prevent any weird returns
    fclose(src);
    fclose(dst);

    return nullptr;
}

int main(int argc, char *argv[]) {

    // There should always be exactly 4 values for argv (filename, string number of threads, source and destination directories)
    if(argc != 4) {
        std::cerr << "Error: Amount of inputs are not equivalent to the expected 4\n";
        return EXIT_FAILURE;
    }
    
    // Converts n string to int
    int n = atoi(argv[1]);

    // Checks to see if N is NOT between 2 and 10
    if(!(n >= 2 && n <= 10)) {
        std::cerr << "Error: N is not within the range of [2,10]\n";
        return EXIT_FAILURE;    
    }
        
    // Buffer to hold metadata
    struct stat file_info; 

    // Checks to see if Source Doesn't Exist
    if(stat(argv[2], &file_info) == -1) {
        std::cerr << "Error: Source directory file not found\n";
        return EXIT_FAILURE;
    }

    // Checks to see if Source directory is NOT A directory
    if (!(S_ISDIR(file_info.st_mode))) {
        std::cerr << "Error: Source is NOT a directory\n";
        return EXIT_FAILURE;
    }

    // Checks to see if Destination Doesn't Exist
    if(stat(argv[3], &file_info) == -1) {
        std::cerr << "Error: Destination directory file not found\n";
        return EXIT_FAILURE;
    }

    // Checks to see if Destination directory is NOT A directory
    if (!(S_ISDIR(file_info.st_mode))) {
        std::cerr << "Error: Destination is NOT a directory\n";
        return EXIT_FAILURE;
    }

    // Declare arrays for threads handlers and for threads 
    pthread_t threadHandleArr[10];
    ThreadData threadArr[10];

    // Thread Creation Loop
    for(int i=0;i<n;i++) {
        snprintf(threadArr[i].source_path, sizeof(threadArr[i].source_path), "%s/source%d.txt", argv[2], i+1);
        snprintf(threadArr[i].dest_path, sizeof(threadArr[i].dest_path), "%s/source%d.txt", argv[3], i+1);

        if (pthread_create(&threadHandleArr[i], nullptr, thread_function, &threadArr[i]) != 0) {
            std::cerr << "Error: Thread Failure Occured at: " << i << "\n";
            return EXIT_FAILURE;
        
        }
    }

    // Thread Joining Loop
    for (int i = 0; i < n; i++) {
        pthread_join(threadHandleArr[i], nullptr);
    }

    return 0;
}
