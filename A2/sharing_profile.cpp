#include<bits/stdc++.h>
using namespace std;


struct mem_access{
    long long addr;
    long long tid;
    long long len;
};

void split_line(string line,struct mem_access * temp){
    char *token = strtok((char *)line.c_str()," ");
  
    int i=0;
    while (token != NULL)
    {
        // printf("%s\n", token);
        if(i==0){
            temp->tid= stoi(token);
            i++;
        }
        else if(i==1){
            temp->addr= stoll(token);
            i++;
        }
        else if(i==2){
            temp->len= stoi(token);
            i++;
        }
        else{
            cout<<"Error: More than 3 words in a line"<<endl;
        }
        token = strtok(NULL, " ");
    }
}
    
fstream file;
long long total_lines=0;

int main(int argc,char ** argv){

    unordered_map<long long,unordered_set<long long>> blocks;
    map<long long,long long>cdf;

    if(argc!=2){
        cout<<"Please Enter the file name"<<endl;
        exit(-1);
    }
    file.open(argv[1],ios::in);


    if (file.is_open()){ 
      string line;
      struct mem_access temp;
      while(getline(file, line)){ 
        //  cout << line << "\n"; 
         split_line(line,&temp);   
         blocks[temp.addr/64].insert(temp.tid);
         total_lines++;
      }

      file.close(); 
   }

   long long count_of_sharing[8]={0};

   for(auto it=blocks.begin();it!=blocks.end();it++){
        count_of_sharing[it->second.size()-1]++;
   }


    // long long temp_total_blocks;
   cout<<"Total_lines: "<<total_lines<<endl;
   cout<<"Total_blocks: "<<blocks.size()<<endl;
   for(int i=0;i<8;i++){
    cout<<"Used by "<<i+1<<" threads: "<<count_of_sharing[i]<<endl;
   }





}