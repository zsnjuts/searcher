template <class T> 
class mystack{

private:
#define MAXSTACKSIZE 1024
	T arr[MAXSTACKSIZE];
	int ptr;

public:
    mystack():ptr(0){}
	bool push(const T& x){
		if (ptr == MAXSTACKSIZE - 1)
			return false;
		else{
			arr[ptr] = x;
			ptr++;
			return true;
		}
    }
	bool pop(){
		if (ptr == 0){
			return false;
		}
		else{
			ptr--;
			return true;
		}
    }
    T top(){ return arr[ptr - 1]; }
    bool empty(){ return ptr == 0 ? true : false; }
    int size(){ return ptr; }
};
