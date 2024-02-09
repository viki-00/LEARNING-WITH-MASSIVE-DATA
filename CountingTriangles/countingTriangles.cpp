//g++  countingTriangles.cpp -std=c++17 -O3 -lpthread -fopenmp -o main
#include <omp.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream> 

using namespace std;
using namespace std::chrono;

// Comparator function to sort pairs according to second value
bool cmp(pair<int, int>& a, pair<int, int>& b)
{
    return a.second < b.second;
}

//list of edges
vector<pair<int, int>> create_undirectgraph(const string str) {

    vector<pair<int, int>> edges;
    cout << "creating dataset... " << str << endl;

    std::ifstream infile(str);
    std::string line;
    while (std::getline(infile, line)) {
        if (line[0] != '#') {
            std::istringstream iss(line);
            int a, b;
            iss >> a >> b;
            //check self-loop
            if(a!=b)
                edges.push_back(make_pair(a, b));
        }
    }
    
    return edges;
}

//Rank List
map<int, int> create_RankList(vector<pair<int, int>> edges) {

    map<int, int> r_list;
    vector<pair<int, int>>  aux;
    map<int, int> r_list_final;

    for(int i=0; i<edges.size(); i++){
        int a = edges[i].first;
        int b = edges[i].second;
        //first node:
            if (r_list.find(a) != r_list.end()) {
                r_list[a] += 1;
            }
            else {
                r_list[a] = 1;
            }
            //second node;
            if (r_list.find(b) != r_list.end()) {
                r_list[b] = 0;
            }
    }

    //Copy key-value pair from Map
    // to vector of pairs
    for (auto& it : r_list) {
        aux.push_back(it);
    }

    // Sort using comparator function, cause Map Structure can't be sorted
    sort(aux.begin(), aux.end(), cmp);

    for (int i = 0; i < aux.size(); i++) {
        r_list_final[aux[i].first] = i;
    }

    return r_list_final;
}


//adjacency list, from undirect graph to direct graph using the rank list
map<int, vector<int>> create_adjList(vector<pair<int, int>> edges) {

    map<int, vector<int>> adj_list;
    map<int, int> r_list = create_RankList(edges);

    for(int i=0; i<edges.size(); i++){
        int a = edges[i].first;
        int b = edges[i].second;
        //first node:
        if (r_list[a] < r_list[b])                 
            adj_list[a].push_back(b);                
        else {
            adj_list[b].push_back(a); 
        }           
    }
    
    //sort neighbour list
    for (std::map<int, vector<int>>::iterator iter = adj_list.begin(); iter != adj_list.end(); ++iter)
    {
        int k = iter->first;
        sort(adj_list[k].begin(), adj_list[k].end());
    }
    cout << "dataset created" << endl;
    return adj_list;
}

//list of vertices cause OpenMp can't do parallel  for on Map Structure
vector<int> vector_vertices(map<int, vector<int>> adj_list) {
    vector<int> vertices;
    for (auto const& i : adj_list)
    {
        int key = i.first;
        vertices.push_back(key);
    }
    return vertices;
}

long long int  counting_triangles(vector<int> vertices, map<int, vector<int>> graph) {
    long long int  count = 0;

    for (int i = 0; i < vertices.size(); i++) {
        int key1 = vertices[i];
        vector<int> v1 = graph[key1];
        for (int j = 0; j < v1.size(); j++) {
            int key2 = v1[j];
            vector<int> v2 = graph[key2];
            vector<int> v3;
            set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(v3));
            long long int  add = v3.size();
            count += add;
        }
    }
    return count;
}



long long int  counting_triangles_par(vector<int> vertices, map<int, vector<int>> graph, int n_threads) {
    long long int  count = 0;
   
    #pragma omp parallel for num_threads(n_threads) reduction(+:count)
    for (int i = 0; i < vertices.size(); i++) {
        int key1 = vertices[i];
        vector<int> v1 = graph[key1];
        for (int j = 0; j < v1.size(); j++) {
            int key2 = v1[j];
            vector<int> v2 = graph[key2];
            vector<int> v3;
            set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(v3));
            long long int add = v3.size();
            count += add;
        }
    }
    return count;

}

void project(string arg) {

    const string str = arg;    

    vector<pair<int, int>> edges = create_undirectgraph(str + ".txt"); //list of edges
    map<int, vector<int>> adj_list = create_adjList(edges); //adjacency list of a direct graph
    vector<int> vertices = vector_vertices(adj_list); // list of nodes of the graph

    //svc file
    ofstream myfile;
    string name_file(str + "_results.csv");
    myfile.open(name_file);
    myfile << "N_threads,Milliseconds,Speed_Up\n";

    // number of triangles
    long long int  count = 0;

    // Get starting timepoint
    auto start2 = high_resolution_clock::now();
    count = counting_triangles(vertices, adj_list);
    // Get ending timepoint
    auto stop2 = high_resolution_clock::now();

    cout << "Number of triangles: " << count << endl;

    auto duration2 = duration_cast<milliseconds>(stop2 - start2);

    cout << "Time sequential Algorithm: " << to_string(duration2.count()) << endl;


    cout << "Counting triangles with parallel algorithm..." << endl;
    for (int i = 1; i <= 20; i++) {
        //cout<< "thread n: " <<i<< endl;
        long count2 = 0;
        // Get starting timepoint
        auto start1 = high_resolution_clock::now();
        count2 = counting_triangles_par(vertices, adj_list, i);
        // Get ending timepoint
        auto stop1 = high_resolution_clock::now();

        //cout << "Number of triangles: " << count << endl;


        auto duration1 = duration_cast<milliseconds>(stop1 - start1);

        float speedup = (duration2.count() * 1.0) / (duration1.count() * 1.0);
        //cout << "Time Parallel Algorithm: " << to_string(duration1.count()) << endl;
        //cout << i << " speedup: " << speedup << endl;
        myfile << i << ", "
            << duration1.count() << ", "
            << speedup << "\n";

    }

    myfile.close();



}

int main()
{
    vector <string> arg;

    arg.push_back("as-skitter");
    arg.push_back("facebook_combined");
    arg.push_back("fullCon");
    arg.push_back("edges");  
    arg.push_back("as20000102");  



    for (int i = 0; i < arg.size(); i++) {
        project(arg[i]);
    }

    return 0;
}


