/* be sure to -I the dir where these files are... */
#include <pthread_sync.h>
#include <hashtbl.C>
#include <stdio.h>

using namespace std;

int main(int c, char* v[]) {
    hashtbl<int, const char*, pthread_sync> h;
    h.put(17, "seventeen");
    h.put(37, "thirty-seven");
    h.put(42, "forty-two");
    h.put(32, "thirty-two");
    h.put(22, "twenty-two");

    fprintf(stderr, "%d -> \"%s\"\n", 17, h.get(17));
    fprintf(stderr, "%d -> \"%s\"\n", 37, h.get(37));
    fprintf(stderr, "%d -> \"%s\"\n", 42, h.get(42));
    fprintf(stderr, "%d -> \"%s\"\n", 32, h.get(32));
    fprintf(stderr, "%d -> \"%s\"\n", 22, h.get(22));
    
    h.put(17, "Seventeen");
    h.put(37, "Thirty-Seven");
    h.put(42, "Forty-Two");
    h.put(32, "Thirty-Two");
    h.put(22, "Twenty-Two");
    
    fprintf(stderr, "%d -> \"%s\"\n", 17, h.get(17));
    fprintf(stderr, "%d -> \"%s\"\n", 37, h.get(37));
    fprintf(stderr, "%d -> \"%s\"\n", 42, h.get(42));
    fprintf(stderr, "%d -> \"%s\"\n", 32, h.get(32));
    fprintf(stderr, "%d -> \"%s\"\n", 22, h.get(22));

    return 0;
}
