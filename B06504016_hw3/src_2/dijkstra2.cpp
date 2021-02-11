#include<iostream>
#include<fstream>
#include <string>
#include<bits/stdc++.h>
#include<sstream>
using namespace std;
int n;
int FindMinIndex(int *R, bool *visited){
    int j = 0, k = 999999 ;
    for(int i=0;i<n;i++){
        if(k>R[i] && R[i] > 0 && visited[i] == 0){
            k=R[i];
            j=i;
        }
    }
    if(k == 999999){
        return -1;
    }
    else{
        return j;
    }
}
void relax(int node_start, int node_end, int **R, int *L, int *S){
    if(S[node_end] > S[node_start] + R[node_start][node_end] || S[node_end] == -1){
        S[node_end] = S[node_start] + R[node_start][node_end];
        L[node_end] = node_start;
    }
}
void Dijkstra(int **R,int *L, int *S, int source){
    bool visited[n] = {0};
    int counting = 0;
    int next;
    L[source] = source;
    for(int i=0;i<n;i++){
        if(R[source][i] > 0){
            L[i] = source;
        }
    }
    visited[source] = 1;
    while(counting != n){
        next = FindMinIndex(S, visited);
        visited[next] = 1;
        if(next == -1){
            break;
        }
        for(int i=0;i<n;i++){
           if(R[next][i] > 0 && visited[i] == 0){
                relax(next, i, R, L, S);
           }
        }
        source = next;
        counting++;
    }
}
int find_next_step(int *L, int source, int router){
    if(L[router] == -1){
        return -1;
    }
    else{
        while(L[router] != source){
            router = L[router];
        }
        return router + 1;
    }
}
int main(int argc,char* argv[]){
    string input, first, second;
    int position;
    int cut = -1;
    getline(cin, input);
    first = input.substr(0, 2);
    second = input.substr(3);

        if(first == "lf"){
            char infilename[second.size() + 1];
            strcpy(infilename, second.c_str());
            fstream fin(infilename);
            fstream fout;
            position = second.find_first_of(".", 0);
            string output;
            output = second.substr(0, position);
            if(cut == -1){
                output = output + "_out1.txt";
            }
            else{
                output = output + "_out2.txt";
            }
            char outfilename[output.size() + 1];
            strcpy(outfilename, output.c_str());
            fout.open(outfilename, ios::out);
            fin>>n;
            int removed[n];
            int weight;
            int **Routing_Table=NULL;
            Routing_Table=new int*[n];
            for(int i=0;i<n;i++){
                    Routing_Table[i]=new int[n];
                    }
            for(int i=0;i<n;i++){
                removed[i] = 0;
                for(int j=0;j<n;j++){
                    fin>>weight;
                    Routing_Table[i][j] = weight;
                }
            }
            int last_router[n][n];
            int shortest_cost[n][n];
            for(int i=0;i<n;i++){
                        for(int j=0;j<n;j++){
                        last_router[i][j] = -1;
                        shortest_cost[i][j] = Routing_Table[i][j];
                        }
                    }
            while(1){
                getline(cin, input);
                first = input.substr(0, 2);
                if(input.size()> 2 ){
                second = input.substr(3);
                }
                if(first == "rm"){
                    string number;
                    number = second.substr(1);
                    cut = atoi(number.c_str());
                    removed[cut - 1] = 1;
                    for(int i=0;i<n;i++){
                        Routing_Table[i][cut-1] = -1;
                        Routing_Table[cut-1][i] = -1;
                    }

                }
                if(first == "of"){
                    for(int i=0;i<n;i++){
                        Dijkstra(Routing_Table, last_router[i], shortest_cost[i], i);
                    }
                    for(int i=0;i<n;i++){
                        for(int j=0;j<n;j++){
                            if(shortest_cost[i][j] == 0 && i != j ){
                                shortest_cost[i][j] = -1;

                            }
                    }
                }
                    for(int i=0;i<n;i++){
                        if(removed[i] == 0){
                            fout<<"Routing table of router "<<i+1<<endl;
                            for(int j=0;j<n;j++){
                                if(removed[j] == 0){
                                    fout<<shortest_cost[i][j]<<" "<<find_next_step(last_router[i], i, j)<<endl;
                                }
                            }
                        }
                    }

                for(int i=0;i<n;i++){
                    delete Routing_Table[i];
                }
                delete Routing_Table;
                fin.close();
                fout.close();
                break;
                }
        }




        }

        return 0;

}
