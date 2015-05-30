// Compute PageRank for each web pages
// usage: ./compute_page_rank f_link.txt r_link.txt page_content.txt

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <math.h>
using namespace std;

#define PAGE_SIZE 501

typedef struct {
    list<int> f_link;
    list<int> r_link;
} Page;

void read_vec(const string s, char delim, list<int> &link);
void read_link_info(string flink_fname, string rlink_fname, vector<Page> &pages);
void read_page_content(string fname, vector<string> &page_content);
void compute_page_rank(const vector<Page> &pages, const float d, const float DIFF, vector<float> &pr);

int main(int argc, char** argv)
{
    //Set up link info and page content file name
    string flink_fname = argv[1];
    string rlink_fname = argv[2];
    string page_content_fname = argv[3];

    //Prompt user to enter parameters for PageRank Algorithm
    float d;
    float DIFF;
    cout << "Enter d: ";
    cin >> d;
    cout << "Enter DIFF: ";
    cin >> DIFF;

    //Read in page link and page content
    vector<Page> pages(PAGE_SIZE);
    vector<string> page_content(PAGE_SIZE);
    read_link_info(flink_fname, rlink_fname, pages);
    read_page_content(page_content_fname, page_content);

    //Calculate PageRank for every pages
    vector<float> page_rank(PAGE_SIZE);
    compute_page_rank(pages, d, DIFF, page_rank);

    //Print the calculated PageRank
    for(int i = 0; i < page_rank.size(); i++)
        cout << page_rank[i] << endl;

    //Store the page rank file - PageRank

    //Store the reverse index file - ReverseIndex

}


void read_vec(const string s, char delim, list<int> &link)
{
    stringstream ss(s);
    string item;
    while( getline(ss, item, delim) ){
        link.push_back( stoi(item) );
    }
}

void read_link_info(string flink_fname, string rlink_fname, vector<Page> &pages)
{
    //Read in f_link
    ifstream f;
    f.open(flink_fname, ios::in);
    if( f.is_open() ){
        string line;
        int idx = 0;
        while( getline(f,line) ){
            read_vec(line, ' ', pages[idx].f_link);
            idx++;
        }
        f.close();
    }

    //Read in r_link
    f.open(rlink_fname, ios::in);
    if( f.is_open() ){
        string line;
        int idx = 0;
        while( getline(f, line) ){
            read_vec(line, ' ', pages[idx].r_link);
            idx++;
        }
        f.close();
    }
}

void read_page_content(string fname, vector<string> &page_content)
{
    ifstream f;
    f.open(fname, ios::in);
    if( f.is_open() ){
        string line;
        int idx = 0;
        while( getline(f, line) ){
            page_content[idx] = line;
            idx++;
        }
        f.close();
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
