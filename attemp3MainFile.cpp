
#include <thread>
#include <fstream>
#include <vector>
#include <iostream>
#include <mutex> 

using namespace std;

//GLOBALS
struct element
{
    int value;
    int index;
};
element elements;
vector<thread> threadpool;  //this will spawn threads, vector of threads
vector<element> smolpool;   //where the min values and index of each thread that ran the findMin function are stored
mutex tcontrol;

vector<int> values; //vector to store values from input file
int inputCount = 0; //count of values in the input file

void readFile(char * argv[]);    //function that reads the values from the input files in 
void writetoFile();     //function that writes the vector values out to a txt file
void printElement(); //for debugging purposes

void compare(int index1, int index2);   //compares two values
int findMin(int start, int end);   // will look through the val vector to find the smallest value given indices

int main(int argc, char * argv[]) 
{
    //main has a loop that spawns threads and assigns the left and right end points of the list to process
    //this PC can run 8 threads in parallel
    const int maxThreads = thread::hardware_concurrency();

    readFile(argv); 

    int numThread = 0;  //how many values a thread will deal with
    int temp = 0;
    int threadCounter = 0;  //keeps track of the threads
    int listIndice = 0; //keeps track of where the in the vector indice wise we are swapping with once we've found the minimum in the sub vector

    while(listIndice < inputCount)  
    {
        numThread = (inputCount-listIndice)/maxThreads; //how many values each thread will deal with initially

        temp = 0;   //temp needs to be resetted after every loop as it will change when listIndice changes
        threadCounter = 0;  //keeps track of how many threads have been spawned also for debugging
        if((inputCount-listIndice) % maxThreads != 0)   //how many value the last few threads need to run, it will need to work a lil more
        {
            temp = maxThreads - ((inputCount-listIndice) % maxThreads); //temp tells us on which thread will the threads have to take on more values
        }
        for(int i = listIndice; i < inputCount; i += numThread)  //will spawn 8 threads
        {
            if(temp > 0 && threadCounter >= temp)   //a temp value has to exist first and the thread count should be higher since its the last few
            {                                               //that deal with more values
                threadpool.push_back(thread(findMin, i, i+(numThread+1)));    //add one more to element for the remaining threads to run
                i++;    //adding one more means you also have to adjust i
            }
            else
            {   
                threadpool.push_back(thread(findMin, i, i+numThread));     //thread run the findMin function given the start and end
            }
            threadCounter++;    //how many times a thread has been spawned
        }
        
        for(int i = 0; i < threadCounter; i++)  //wait for all threads to finish
            threadpool[i].join(); //threads are finished 
        
        //clear the vector, deletes the threads that have been spawn and start new
        threadpool.clear(); //THIS IS IMPORTANT TO DO 


        while(smolpool.size() > 1)
        {
            threadCounter = 0;  //there should be no more threads spawned at the beginning of this loop
            for(int i = 0; i < smolpool.size()-1; i++)  //smolpool.size() will change as threads run
            {
                    threadpool.push_back(thread(compare, i, i+1));  //comparing two values
                    threadpool[threadCounter].join();   //this is necessary here bc the vector is constantly changing depending on how fast the threads run so a thread may go out of bounds 10/8/2020
                    threadCounter++;
                    i++;
            }

            //for(int i = 0; i < threadCounter; i++)
            //    threadpool[i].join(); //threads are finished 

            threadpool.clear(); //THIS IS IMPORTANT TO DO
            threadCounter = 0;
        }
        //now you want to swap whatevers left in the smaller vector with the an indice
        int tempV = 0;

        tempV = values.at(listIndice);  //the place at which were swapping with
        values.at(listIndice) = (smolpool.at(0)).value; //should always be one element left in vector
        values.at((smolpool.at(0)).index) = tempV;
     
        smolpool.clear();

        listIndice++;    //will keep track of where swap
    }

    writetoFile();
}

/** function_identifier: read the passed in text file via argv into the values vector
 * * parameters: argument vector
 * * return value: none*/
void readFile(char * argv[])
{
    ifstream iFile;
    int val = 0;
    string filename = "";

    /*do
    {
        cout << "Input file name: ";
        cin >> filename;
    }
    while(!iFile.open(filename);*/


    iFile.open(argv[1]);    //for me COMMENT OUT WHEN DONE

    while(!iFile.eof()) //this condition nevers works the way u want it to
    {
         
        iFile >> val;//reading in from input file
        values.push_back(val);  //without this i will get errors idk why
        inputCount++;
    }
    iFile.close();
}

/** function_identifier: writing the values vector to an output file
 * * parameters: none
 * * return value: none*/
void writetoFile()
{
    ofstream oFile;

    oFile.open("sortedLIST.txt");

    for (auto& it : values) 
    { 
        oFile << it << endl; 
    } 

    oFile.close();
}

/** function_identifier: sets the base for the minimum values, and then runs a loop to compare the values
 *  in the values array to that minimum base value, and whichever value is smaller gets set as minimum
 * * parameters: start and end indice of the vector to iterate through
 * * return value: NOT NECESSARY but returns an int (dont wanna change in case it messes up program)*/
int findMin(int start, int end)
{ 
    tcontrol.lock();    //starts accessing the global vector
    int min = values.at(start); //setting the base for the min values 
    elements.value = values.at(start);
    elements.index = start;

    int i;
    
    for(i = start; i < end; i++)    //from the start indice to the end indice 
    {

        if(values.at(i) < min)  //find the smallest value of bwtn those indices
        {
            min = values.at(i); //set that value to the vector of structs
            elements.index = i;
            elements.value = min;
        }  
    }
    
    smolpool.push_back(elements);   //save it into the vector
    tcontrol.unlock();

    //return 1;
}

/** function_identifier: prints the elements in the smaller array 
 * * parameters: none
 * * return value: none*/
void printElement()
{
    for (auto& it : smolpool) 
    { 
        cout << "Printvalue: " << it.value << " Printindex: " << it.index << endl; 
    } 
}

/** function_identifier: compares to values at the passed in indices
 * * parameters: indices that should be right next to each other 
 * * return value: none*/
void compare(int index1, int index2)
{   
    tcontrol.lock();
    if((smolpool.at(index1)).value < (smolpool.at(index2)).value)
    {
        smolpool.erase(smolpool.begin()+index2);
    }
    else //if((smolpool.at(index1)).value > (smolpool.at(index2)).value)
    {
        smolpool.erase(smolpool.begin()+index1);

    }
    tcontrol.unlock();
}
