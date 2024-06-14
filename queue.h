#ifndef QUEUE_H
#define QUEUE_H

class queue
{
private:
    unsigned char *arr;
    unsigned short capacity;
    unsigned short headIndex;
    unsigned short tailIndex; //this indicates the index AFTER the last filled index
    unsigned short numEntries;
public:
    queue(unsigned short);
    ~queue();
    unsigned char push(unsigned char);
    unsigned char pop();
    unsigned char isEmpty();
    unsigned char isFull();
};

queue::queue(unsigned short cap)
{
    if(cap == 0) cap++; //idk what it would do with 0 but prolly smth bad
    arr = new unsigned char[cap];
    capacity = cap;
    headIndex = 0;
    tailIndex = 0;
    numEntries = 0;
}

queue::~queue()
{
    delete[] arr;
}

unsigned char queue::isEmpty(){
    return(numEntries == 0);
}
unsigned char queue::isFull(){
    return(numEntries == capacity);
}

/// @return returns 0 if unable to push, else 1
unsigned char queue::push(unsigned char data){
    if(!isFull()){
        arr[tailIndex] = data;
        tailIndex = (tailIndex + 1) % capacity;
        numEntries++;
        return( 1);
    }
    return( 0);
}

/// @return returns 0 if not able to pop, else returns tail entry
unsigned char queue::pop(){
    if(!isEmpty()){
        unsigned char entry = arr[headIndex];
        headIndex = (headIndex + 1) % capacity;
        numEntries--;
        return(entry);
    }
    return(0);
}

#endif
