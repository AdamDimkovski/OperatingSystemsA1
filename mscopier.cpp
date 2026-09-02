#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fstream>
#include <queue>
#include <string>

using namespace std;

// Setting constants
#define MAX_QUEUE_SIZE 20 // max number of blocks to be stored in the queue
#define BUFFER_SIZE 4096 // number of bytes each reader can attempt to read

// Shared queue and files
queue<string> dataQueue; //Shared queue to transfer from reader to writer threads
ifstream sourceFile; // Where the reader reads from
ofstream destinationFile; // Where the writer writes to 

// A variable to say when the readers are finished reading
bool readingFinished = false;

pthread_mutex_t queueMutex; // ensure the safety of thread by locking dataQueue and readingFinished access

// Reader thread
void* reader(void* data){
    int id = *((int*)data); // Giving an ID to each reader thread
    printf("Reader %d started\n", id); // Prints when each reader thread starts
    
    while (true){
        char buffer[BUFFER_SIZE];

        sourceFile.read(buffer, BUFFER_SIZE); // Reads BUFFER_SIZE bytes into buffer from sourcefile
        streamsize bytesRead = sourceFile.gcount(); // How many bytes are read 
        
        if (bytesRead <= 0){ // Checks if their are blocks left in the file if so stops reading
            break;
        }

        string block(buffer, bytesRead); // Store bytes read from the file as a string block for the shared queue

        // Lock before accessing shared queue
        if (pthread_mutex_lock(&queueMutex) != 0){
            fprintf(stderr, "Reader %d: mutex lock failed\n", id);
            break;
        } 

        // Checks whether the queue has space for more blocks then prints when block is added
        if (dataQueue.size() < MAX_QUEUE_SIZE){
            dataQueue.push(block);
            printf("Reader %d added data\n", id);
        }

        // Unlock queue as soon as operations complete
        if (pthread_mutex_unlock(&queueMutex) != 0){
            fprintf(stderr, "Reader %d: mutex unlock failed\n", id);
            break;
        }

    }

    printf("Reader %d finished\n", id); // Prints when reader thread is finished
    pthread_exit(nullptr);
}

// Writer thread
void* writer(void* data){
    int id = *((int*)data);
    printf("Writer %d started\n", id);
    while (true){

        // Lock before reading or updating the shared queue
        if (pthread_mutex_lock(&queueMutex) != 0){
            fprintf(stderr, "Writer %d: mutex lock failed\n", id);
            break;
        }

        // Checks whether the queue has data
        if (!dataQueue.empty()){

            // Get the first item and Remove it
            string block = dataQueue.front();
            dataQueue.pop();

            // Unlock before the slow file write so that other threads are able to utilise queue aswell
            if (pthread_mutex_unlock(&queueMutex) != 0){
                fprintf(stderr, "Writer %d: mutex unlock failed\n", id);
                break;
            }

            // Write it to the destination
            destinationFile.write(block.c_str(), block.size());
            printf("Writer %d wrote data\n", id);
        }

        else {
            
            // Queue is empty: check the shared finished flag before unlock
            bool done = readingFinished;

            // Unlock before proceeding to decision
            if (pthread_mutex_unlock(&queueMutex) != 0){
                fprintf(stderr, "Writer %d: mutex unlock failed\n", id);
                break;
            }

            // Exit loop if readers are complete and queue is empty
            if (done) {
                break;
            }
        }
    }

    printf("Writer %d finished\n", id);
    pthread_exit(nullptr);
}

int main(int argc, char* argv[]){
    // Check command-line arguments
    if (argc != 4){
        printf("Usage: ./mscopier n source_file destination_file\n");
        return 1;
    }

    // Get number of readers/writers
    int n = atoi(argv[1]);

    // Check that n is valid
    if (n < 2 || n > 10){
        printf("n must be between 2 and 10\n");
        return 1;
    }


    // Get file names and Open source file
    string sourceName = argv[2];
    string destinationName = argv[3];
    sourceFile.open(sourceName, ios::binary);

    if (!sourceFile){
        printf("Could not open source file\n");
        return 1;
    }

    // Open destination file
    destinationFile.open(destinationName, ios::binary);
    if (!destinationFile) {
        printf("Could not open destination file\n");
        return 1;
    }

    // Initialise mutex before creating additonal threads, and check return value
    if (pthread_mutex_init(&queueMutex, nullptr) != 0) {
        fprintf(stderr, "Failed to initialise mutex\n");
        return 1;
    }

    //Thread arrays
    pthread_t readers[10];
    pthread_t writers[10];

    // IDs for each thread
    int readerIDs[10];
    int writerIDs[10];

    // Create reader threads
    for (int i = 0; i < n; i++){
        readerIDs[i] = i + 1;
        if (pthread_create(&readers[i], nullptr, reader, &readerIDs[i]) != 0){
            fprintf(stderr, "Failed to create reader thread %d\n", i + 1);
            return 1;
        }
    }

    // Create writer threads
    for (int i = 0; i < n; i++){
        writerIDs[i] = i + 1;
        if (pthread_create(&writers[i], nullptr, writer, &writerIDs[i]) != 0){
            fprintf(stderr, "Failed to create writer thread %d\n", i + 1);
            return 1;
        }
    }

    // Wait for all readers to finish
    for (int i = 0; i < n; i++){
        pthread_join(readers[i], nullptr);
    }
    
    // Tell writers that no more data will be added
    if (pthread_mutex_lock(&queueMutex) != 0){
        fprintf(stderr, "main: mutex lock failed\n");
        return 1;
    }
    readingFinished = true;
    if (pthread_mutex_unlock(&queueMutex) != 0){
        fprintf(stderr, "main: mutex unlock failed\n");
        return 1;
    }

    // Wait for all writers to finish
    for (int i = 0; i < n; i++){
        pthread_join(writers[i], nullptr);
    }

    // Destroy mutex after all threads complete
    if (pthread_mutex_destroy(&queueMutex) != 0){
        fprintf(stderr, "Failed to destroy mutex\n");
        return 1;
    }

    // Close files
    sourceFile.close();
    destinationFile.close();

    printf("File copying complete\n");

    return 0;
}