#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fstream>
#include <queue>
#include <string>

using namespace std;

#define QUEUE_SIZE 20
#define BUFFER_SIZE 4096

queue<string> dataQueue;

// Shared source and destination files
ifstream sourceFile;
ofstream destinationFile;

bool readingFinished = false;

// Reader thread
void* reader(void* data){
    int id = *((int*)data);
    printf("Reader %d started\n", id);
    
    while (true){
        char buffer[BUFFER_SIZE];

        sourceFile.read(buffer, BUFFER_SIZE);
        streamsize bytesRead = sourceFile.gcount();

        if (bytesRead <= 0){
            break;
        }

        string block(buffer, bytesRead);
        if (dataQueue.size() < QUEUE_SIZE){
            dataQueue.push(block);

            printf("Reader %d added data\n", id);
        }
    }

    printf("Reader %d finished\n", id);
    pthread_exit(NULL);
}

// Writer thread
void* writer(void* data){
    int id = *((int*)data);
    printf("Writer %d started\n", id);
    while (true){

        // Check whether the queue has data
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
    pthread_exit(NULL);
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
        pthread_create(&readers[i], NULL, reader, &readerIDs[i]);
    }

    // Create writer threads
    for (int i = 0; i < n; i++){
        writerIDs[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writerIDs[i]);
    }

    // Wait for all readers to finish
    for (int i = 0; i < n; i++){
        pthread_join(readers[i], NULL);
    }
    // Tell writers that no more data will be added
    readingFinished = true;

    // Wait for all writers to finish
    for (int i = 0; i < n; i++){
        pthread_join(writers[i], NULL);
    }

    // Close files
    sourceFile.close();
    destinationFile.close();

    printf("File copying complete\n");

    return 0;
}