// Compute PageRank for each web pages, construct a Reverse Index table and build a simple search engine
// usage: ./compute_page_rank $dir

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

#define PAGE_SIZE 501

typedef struct {
    list<int> f_link;
    list<int> r_link;
    list<string> word_list;
} Page;

void tokenize_line(const string s, char delim, list<string> &token_list);
void read_page_info(string dir_path, vector<Page> &pages);
void compute_page_rank(const vector<Page> &pages, const float d, const float DIFF, vector<float> &pr);
void construct_reverse_index(const vector<Page> &pages, map< string, map<int,string> > &reverse_idx_table);
void get_retrieval_list(const map< string, map<int,string> > &reverse_idx_table, const list<string> &query_list, map<int,string> &and_list, map<int,string> &or_list);
void print_list(const list<string> &l);
void print_by_pr_order(const vector<float> &pr, const map<int, string> &m);
bool pair_compare(const pair<float,string> &first_pair, const pair<float,string> &second_pair);
void dump_page_rank_list(const vector<Page> pages, const vector<float> &pr);
void dump_reverse_index_table(const map< string, map<int,string> > &reverse_idx_table);

int main(int argc, char** argv)
{
    //Set up link info and page content file name
    string dir_path = argv[1];
    if( dir_path[dir_path.size()-1] != '/' )
        dir_path = dir_path + "/";

    //Prompt user to enter parameters for PageRank Algorithm
    float d;
    float DIFF;
    string parameter;
    cout << "Enter d: ";
    cin >> d;
    cout << "Enter DIFF: ";
    cin >> DIFF;
    cin.clear();
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    //Read in page link and page content
    cout << "Reading page info from files ... " << endl;
    vector<Page> pages(PAGE_SIZE);
    read_page_info(dir_path, pages);

    //Calculate PageRank for every pages and Construct reverse index
    cout << "Calculating PageRank for every pages ..." << endl;
    vector<float> page_rank(PAGE_SIZE);
    compute_page_rank(pages, d, DIFF, page_rank);

    //Construct Reverse Index
    cout << "Constructing Reverse Index ... " << endl;
    map< string, map<int,string> > reverse_idx_table;
    construct_reverse_index(pages, reverse_idx_table);

    //Store the page rank file - PageRank
    dump_page_rank_list(pages, page_rank);

    //Store the reverse index file - ReverseIndex
    dump_reverse_index_table(reverse_idx_table);

    //Run the search engine
    cout << "Ready for search!\n" << endl;
    list<string> query_list;
    do {
        //Prompt user to enter the query list
        string line;
        query_list.clear();
        cout << "Enter query list: " << endl;
        getline(cin, line);
        tokenize_line(line, ' ', query_list);
        
        //Get the retrieval list
        map<int, string> and_list;
        map<int, string> or_list;
        get_retrieval_list(reverse_idx_table, query_list, and_list, or_list);

        //Print out the retrieval result
        list<string>::iterator it;
        if( query_list.size() == 1 ){
            print_list(query_list);
            if( and_list.size() == 0 )
                cout << "No relevant pages found";
            else
                print_by_pr_order(page_rank, and_list);
            cout << endl;
        }
        else{
            //print and list
            cout << "AND ( ";
            print_list(query_list);
            cout << ") ";
            if( and_list.size() == 0 )
                cout << "No relevant pages found";
            else
                print_by_pr_order(page_rank, and_list);
            cout << endl;

            //print or list
            cout << "OR ( ";
            print_list(query_list);
            cout << ") ";
            if( or_list.size() == 0 )
                cout << "No relevant pages found";
            else
                print_by_pr_order(page_rank, or_list);
            cout << endl;
        }
    } while( query_list.begin()->compare("*end*") != 0 );

    return 0;
}


bool pair_compare(const pair<float,string> &first_pair, const pair<float,string> &second_pair)
{
    return first_pair.first > second_pair.first;
}

void print_list(const list<string> &l)
{
    for(list<string>::const_iterator it = l.begin(); it != l.end(); it++)
        cout << *it << " ";
}

void print_by_pr_order(const vector<float> &pr, const map<int, string> &m)
{
    if( m.size() != 0 ){
        //Bind page_id with its PageRank into pair_vec
        vector< pair<float, string> > pair_vec( m.size() );
        for(map<int,string>::const_iterator it = m.begin(); it != m.end(); it++)
            pair_vec.push_back( pair<float,string>(pr[it->first], it->second) );

        //Sort the pair_vec by descending PageRank order
        sort(pair_vec.begin(), pair_vec.end(), pair_compare);

        //Print out the second element of sorted pair_vec in order
        for(vector< pair<float,string> >::iterator it = pair_vec.begin(); it != pair_vec.end(); it++)
            cout << it->second << " ";
    }
}

void tokenize_line(const string s, char delim, list<string> &token_list)
{
    stringstream ss(s);
    string item;
    while( getline(ss, item, delim) ){
        token_list.push_back(item);
    }
}

void read_page_info(string dir_path, vector<Page> &pages)
{
    //Set up file name list
    vector<string> f_list(pages.size());
    DIR *dp;
    struct dirent *ep;
    dp = opendir(dir_path.c_str());
    
    if (dp != NULL){
        while( (ep = readdir(dp)) ){
            string fname = ep->d_name;
            if( fname == "." || fname == ".." )
                continue;
            int page_id = stoi( fname.substr(4) );
            f_list[page_id] = (dir_path + fname);
        }
    }
    else
        perror("Could not open the directory");

    //Read in and parse each page info file
    ifstream f;
    for(int i = 0; i < f_list.size(); i++){
        f.open(f_list[i].c_str(), ios::in);
        if( f.is_open() ){
            string line;
            bool start_content = false;
            while( getline(f, line) ){
                if( line == "---------------------" ){
                    start_content = true;
                    continue;
                }
                if(start_content){
                    tokenize_line(line, ' ', pages[i].word_list);
                }
                else{
                    int link_id = stoi(line.substr(4));
                    pages[i].f_link.push_back(link_id); //push link_id into page i's f_link
                    pages[link_id].r_link.push_back(i); // push i into page link_id's r_link
                }
            }
            f.close();
        }
    }
}

void compute_page_rank(const vector<Page> &pages, const float d, const float DIFF, vector<float> &pr)
{
    //Initialize PageRank for each pages
    float initial_value = 1.0 / float(pr.size());
    for(int i = 0; i < pr.size(); i++)
        pr[i] = initial_value;

    //Updata PageRank while diff >= DIFF
    float diff;
    do {
        //Initialize diff
        diff = 0.0;

        //Copy the old PageRank value
        vector<float> old_pr = pr;

        //Calculate the new PageRank according to previous one
        for(int i = 0; i < pr.size(); i++){
            //Update PageRank
            pr[i] = (1.0-d) / float(pr.size());
            for(list<int>::const_iterator it = pages[i].r_link.begin(); it != pages[i].r_link.end(); it++)
                pr[i] += d * pr[*it] / float(pages[*it].f_link.size());

            //Update diff
            diff += fabs(old_pr[i] - pr[i]);
        }
    } while( diff >= DIFF );
}

void construct_reverse_index(const vector<Page> &pages, map< string, map<int,string> > &reverse_idx_table)
{
    for(int i = 0; i < pages.size(); i++){
        string page_name = "page" + to_string(i);
        for(list<string>::const_iterator it = pages[i].word_list.begin(); it != pages[i].word_list.end(); it++){
            map< string, map<int,string> >::iterator rit;
            rit = reverse_idx_table.find(*it);
            if( rit == reverse_idx_table.end() ){
                map<int,string> tmp;
                tmp.insert( pair<int,string>(i, page_name) );
                reverse_idx_table.insert( pair< string, map<int,string> >(*it, tmp) );
            }
            else
                rit->second.insert( pair<int,string>(i, page_name) );
        }
    }
}

void get_retrieval_list(const map< string, map<int,string> > &reverse_idx_table, const list<string> &query_list, map<int,string> &and_list, map<int, string> &or_list)
{
    //Clear AND list and OR list
    and_list.clear();
    or_list.clear();

    for(list<string>::const_iterator it = query_list.begin(); it != query_list.end(); it++){
        //Initialize iterator and candidate page list containing the query
        map<string, map<int,string> >::const_iterator tmp_it = reverse_idx_table.find(*it);
        map<int,string> candidate_table;
        if( tmp_it != reverse_idx_table.end() )
            candidate_table = tmp_it->second;
        else
            continue;
        map<int,string>::const_iterator cit;

        //OR list: push back all found pages
        for(cit = candidate_table.begin(); cit != candidate_table.end(); cit++)
                or_list.insert( pair<int,string>(cit->first, cit->second) );

        //AND list: Push back all the candidate pages for first query and erase the one not containing following queries
        if( it == query_list.begin() ){
            for(cit = candidate_table.begin(); cit != candidate_table.end(); cit++)
                and_list.insert( pair<int,string>(cit->first, cit->second) );
        }
        else if( and_list.size() > 0 ){
            for(map<int,string>::iterator ait = and_list.begin(); ait != and_list.end(); ait++)
                if( candidate_table.find(ait->first) == candidate_table.end() )
                    and_list.erase(ait);
        }
    }
}

void dump_page_rank_list(const vector<Page> pages, const vector<float> &pr)
{
    ofstream f;
    f.open("PageRank.txt", ios::out);
    for(int i = 0; i < pr.size(); i++){
        string page_name = "page" + to_string(i);
        int n_flink = pages[i].f_link.size();
        string line = ( page_name + " " + to_string(n_flink) + " " + to_string(pr[i]) + "\n");
        f << line;
    }
    f.close();
}

void dump_reverse_index_table(const map< string, map<int,string> > &reverse_idx_table)
{
    ofstream f;
    f.open("Reverse_index.txt", ios::out);
    for(map< string, map<int,string> >::const_iterator it = reverse_idx_table.begin(); it != reverse_idx_table.end(); it++){
        string line = it->first;
        for(map<int,string>::const_iterator lit = it->second.begin(); lit != it->second.end(); lit++)
            line = (line + " " + lit->second);
        line = line + "\n";
        f << line;
    }
    f.close();
}
