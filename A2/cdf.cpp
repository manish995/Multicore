#include<bits/stdc++.h>
using namespace std;


struct mem_access{
    long long addr;
    int tid;
    int len;
};

void split_line(string line,struct mem_access * temp){
    char *token = strtok((char *)line.c_str()," ");
  
    // Keep printing tokens while one of the
    // delimiters present in str[].
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

int main(int argc,char ** argv){

    unordered_map<long long,vector<int>> blocks;
    map<long long,long long>cdf;

    if(argc!=2){
        cout<<"Please Enter the file name"<<endl;
        exit(-1);
    }
    file.open(argv[1],ios::in);
    int line_num=1;
    if (file.is_open()){ 
      string line;
      struct mem_access temp;
      while(getline(file, line)){ 
        //  cout << line << "\n"; 
         split_line(line,&temp);   
         blocks[temp.addr/64].push_back(line_num);
         line_num++;     
      }
      file.close(); 
   }


    cout<<"Unique blocks: "<<blocks.size()<<endl;


   for(auto it=blocks.begin();it!=blocks.end();it++){
    vector<int> curr_block_access=it->second;
    for(int i=0;i<curr_block_access.size()-1;i++){
        cdf[curr_block_access[i+1]-curr_block_access[i]]++;
    }
   }

    // set<pair<long long,long long>> sorted_cdf;

   
   for(auto it=cdf.begin();it!=cdf.end();it++){
    cout<<it->first<<" "<<it->second<<endl;
    
   }

}