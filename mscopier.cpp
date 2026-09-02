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

        // Checks whether the queue has space for more blocks then prints when block is added
        if (dataQueue.size() < MAX_QUEUE_SIZE){
            dataQueue.push(block);
            printf("Reader %d added data\n", id);
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

        // Checks whether the queue has data
        if (!dataQueue.empty()){

            // Get the first item and Remove it
            string block = dataQueue.front();
            dataQueue.pop();

            // Write it to the destination
            destinationFile.write(block.c_str(), block.size());
            printf("Writer %d wrote data\n", id);
        }

        else if (readingFinished){
            break;
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

    //Thread arrays
    pthread_t readers[n];
    pthread_t writers[n];

    // IDs for each thread
    int readerIDs[n];
    int writerIDs[n];

    // Create reader threads
    for (int i = 0; i < n; i++){
        readerIDs[i] = i + 1;
        pthread_create(&readers[i], nullptr, reader, &readerIDs[i]);
    }

    // Create writer threads
    for (int i = 0; i < n; i++){
        writerIDs[i] = i + 1;
        pthread_create(&writers[i], nullptr, writer, &writerIDs[i]);
    }

    // Wait for all readers to finish
    for (int i = 0; i < n; i++){
        pthread_join(readers[i], nullptr);
    }
    
    // Tell writers that no more data will be added
    readingFinished = true;

    // Wait for all writers to finish
    for (int i = 0; i < n; i++){
        pthread_join(writers[i], nullptr);
    }

    // Close files
    sourceFile.close();
    destinationFile.close();

    printf("File copying complete\n");

    return 0;
}