//Name: William Hunt

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

class Classifier{
  public:
    map<string,int> dictionary;
    int totalMessages = 0;
    int totalWords = 0;
    map<string,pair<double,double>> probabilities;
    double ProbHam = 0;
    double ProbSpam = 0;
    int wordsHam = 0;
    int wordsSpam = 0;
    double trueNegative = 0;
    double truePositive = 0;
    double falsePositive = 0;
    double falseNegative = 0;

    void getDictionary(string name);
    void getTotalWords();
    void getTotalMessages(string name);
    size_t findPos(string word,string delimiter);
    void printDictionary();
    void printProbabilities();
    void mergeDictionary(map<string,int> &dict);
    void calculateProbabilities(map<string,int> &dict1, map<string,int> &dict2, map<string,int> &alldict, int smoothing, int words1, int words2);
    void classProbCalc(int smoothing, int messages1, int messages2);
    void classifyFileHam(string name,map<string,int> &dict1, map<string,int> &dict2);
    void classifyFileSpam(string name,map<string,int> &dict1, map<string,int> &dict2);

};

/*void Classifier::getDictionary(string name){
  ifstream fileName (name);
  string word;
  string line;
  string delimiter = " ";
  size_t pos = 0;
  vector<string> fileWords;
  while(getline(fileName, line)){
    while( (pos = findPos(line , delimiter)) != string::npos ){
      word = line.substr(0, pos);
      if(word.length() > 0){
        line.erase(0,pos + 1);
        fileWords.push_back(word);
      }else{
        line.erase(0,pos+1);
      }
      if( (pos = findPos(line, delimiter)) == string::npos){
        if(line.length() > 0){
          fileWords.push_back(line);
        }
      }
    }
  }
  for(int i = 0; i < (int)fileWords.size(); i++){
    this->dictionary[fileWords[i]] += 1;
  }
  fileWords.clear();
}
*/
void Classifier::getDictionary(string name){
  ifstream file(string(name), ios_base::in);
  int lineCount = 0;
  if (!file.is_open()) {
    cerr << "Error: cannot open file " << endl;
    exit(1);
  }
  string line;
  while (getline(file, line, ' ')) {
    lineCount++;
    istringstream iss(line);
    string word;
    while (iss >> word) {
      this->dictionary[word]+=1;
    }//while
  }//while
  file.close();
  this->totalMessages = lineCount;
  int sum = 0;
  for(auto it = this->dictionary.begin(); it!=this->dictionary.end();it++){
    sum+=it->second;
  }
  this->totalWords = sum;
}
void Classifier::printDictionary(){
  for(map<string,int>::iterator pos = this->dictionary.begin(); pos != this->dictionary.end(); pos++){
    cout<< "Key: " <<pos -> first << " Value: " << pos -> second <<endl;
  }
}

void Classifier::printProbabilities(){
  for(auto pos = this->probabilities.begin(); pos != this->probabilities.end(); pos++){
    cout<< "Key: " <<pos -> first << " Value: " << pos -> second.first<<" | "<< pos->second.second <<endl;
  }
}

size_t Classifier::findPos(string line, string delimiter){
  size_t position = 0;
  for(int i = 0; i < (int)line.length(); i++){
    if((int)line[i] == (int)delimiter[0]){
      position = i;
      return position;
    }//checks for position of delimiter and returns position.
    else if ((int)line[i] < 0){
      position = i;
      return position;
    }//checks for ascii code below 0 and returns position in string.
    else if ((int)line[i] == 10){
      position = i;
      return position;
    }
  }
  return string::npos;
}//returns the position of a given delimiter 1 of 1 char in length and also checks
// for characters that have an ascii code below 0, if none found returns npos.

void Classifier::getTotalMessages(string name){
  ifstream fileName (name);
  string message;
  int totalLines = 0;
  while(getline(fileName, message)){
    if(message.length() > 1){
      totalLines += 1;
    }
  }
  this->totalMessages = totalLines;
}

void Classifier::getTotalWords(){
  for(map<string,int>::iterator pos = this->dictionary.begin(); pos != this->dictionary.end(); pos++){
    this->totalWords += pos->second;
  }
}

void Classifier::mergeDictionary(map<string,int> &dict){
  for(auto pos = dict.begin(); pos != dict.end(); pos++){
    this->dictionary[pos->first] += pos->second;
  }
}

void Classifier::calculateProbabilities(map<string,int> &dict1, map<string,int> &dict2, map<string,int> &alldict, int smoothing, int words1, int words2){
  for(auto pos = alldict.begin(); pos != alldict.end(); pos++){
    if(dict1.count(pos->first) == 0){
      double denominator = (words1+(smoothing*(int)alldict.size()) );
      double numerator = (smoothing);
      double Hamresult = double(numerator/denominator);
      probabilities[pos->first].first = Hamresult;
    }else{
      double denominator = (words1+(smoothing*(int)alldict.size()));
      double numerator = (dict1[pos->first]+smoothing);
      double Hamresult = double(numerator/denominator);
      probabilities[pos->first].first = Hamresult;
    }
    if(dict2.count(pos->first)==0){
      double denominator = (words2+(smoothing*(int)alldict.size()));
      double numerator = (smoothing);
      double Spamresult = double(numerator/denominator);
      probabilities[pos->first].second = Spamresult;
    }else{
      double denominator = (words2+(smoothing*(int)alldict.size()));
      double numerator = (dict2[pos->first]+smoothing);
      double Spamresult = double(numerator/denominator);
      probabilities[pos->first].second = Spamresult;
    }
  }
}

void Classifier::classProbCalc(int smoothing, int messages1, int messages2){
  double numerator = (messages1 + smoothing);
  double denominator = ((messages1+messages2)+(smoothing*2));
  double result = numerator/denominator;
  this->ProbHam = result;
  numerator = (messages2 + smoothing);
  denominator = ((messages1+messages2)+(smoothing*2));
  result = numerator/denominator;
  this->ProbSpam = result;
}

void Classifier::classifyFileHam(string name,map<string,int> &dict1, map<string,int> &dict2){
  ifstream fileName (name);
  if(!fileName.is_open()){
    cerr<<"file not open"<<endl;
    exit(1);
  }
  string word;
  string line;
  string delimiter = " ";
  size_t pos = 0;
  while(getline(fileName, line)){
    if(line.length() > 1){
      double logPHam = log(this->ProbHam);
      double logPSpam = log(this->ProbSpam);
      while( (pos = findPos(line , delimiter)) != string::npos ){
        word = line.substr(0, pos);
        if(word.length() > 0){
          if((dict1.count(word) == 0) && (dict2.count(word) == 0)){
            //neither have it.
            logPHam+=log(1.0/(this->wordsHam+this->wordsSpam));
            logPSpam+=log(1.0/(this->wordsHam+this->wordsSpam));
          }else if((dict1.count(word) == 0) && (dict2.count(word) != 0)){
            //ham doesn't have it.
            logPHam+=log(1.0/(this->wordsHam+this->wordsSpam));
            logPSpam+=log(this->probabilities[word].second);
          }else if((dict1.count(word) != 0) && (dict2.count(word) == 0)){
            //spam doesn't have it
            logPHam+=log(this->probabilities[word].first);
            logPSpam+=log(1.0/(this->wordsHam+this->wordsSpam));
          }else{
            //both have it
            logPHam+=log(this->probabilities[word].first);
            logPSpam+=log(this->probabilities[word].second);
          }
          line.erase(0,pos + 1);
        }else{
          line.erase(0,pos+1);
        }
        if( (pos = findPos(line, delimiter)) == string::npos){
          if(line.length() > 0){
            if((dict1.count(line) == 0) && (dict2.count(line) == 0)){
              logPHam+=log(1.0/(this->wordsHam+this->wordsSpam));
              logPSpam+=log(1.0/(this->wordsHam+this->wordsSpam));
            }else if((dict1.count(line) == 0) && (dict2.count(line) != 0)){
              logPHam+=log(1.0/(this->wordsHam+this->wordsSpam));
              logPSpam+=log(this->probabilities[line].second);
            }else if((dict1.count(line) != 0) && (dict2.count(line) == 0)){
              logPHam+=log(this->probabilities[line].first);
              logPSpam+=log(1.0/(this->wordsHam+this->wordsSpam));
            }else{
              logPHam+=log(this->probabilities[line].first);
              logPSpam+=log(this->probabilities[line].second);
            }
          }
        }
      }
      if( logPHam > logPSpam ){
        cout<<"ham "<<logPHam<<" "<<logPSpam<<endl;
        this->truePositive+=1;
      }else{
        cout<<"spam "<<logPHam<<" "<<logPSpam<<endl;
        this->falseNegative+=1;
      }
    }
  }
  fileName.close();
}

void Classifier::classifyFileSpam(string name,map<string,int> &dict1, map<string,int> &dict2){
  ifstream fileName (name);
  if(!fileName.is_open()){
    cerr<<"file not open"<<endl;
    exit(1);
  }
  string word;
  string line;
  string delimiter = " ";
  size_t pos = 0;
  while(getline(fileName, line)){
    if(line.length() > 1){
      double logPHam = log(this->ProbHam);
      double logPSpam = log(this->ProbSpam);
      while( (pos = findPos(line , delimiter)) != string::npos ){
        word = line.substr(0, pos);
        if(word.length() > 0){
          if((dict1.count(word) == 0) && (dict2.count(word) == 0)){
            //neither have it.
            logPHam+=log(1.0/(this->wordsHam+this->wordsSpam));
            logPSpam+=log(1.0/(this->wordsHam+this->wordsSpam));
          }else if((dict1.count(word) == 0) && (dict2.count(word) != 0)){
            //ham doesn't have it.
            logPHam+=log(1.0/(this->wordsHam+this->wordsSpam));
            logPSpam+=log(this->probabilities[word].second);
          }else if((dict1.count(word) != 0) && (dict2.count(word) == 0)){
            //spam doesn't have it
            logPHam+=log(this->probabilities[word].first);
            logPSpam+=log(1.0/(this->wordsHam+this->wordsSpam));
          }else{
            //both have it
            logPHam+=log(this->probabilities[word].first);
            logPSpam+=log(this->probabilities[word].second);
          }
          line.erase(0,pos + 1);
        }else{
          line.erase(0,pos+1);
        }
        if( (pos = findPos(line, delimiter)) == string::npos){
          if(line.length() > 0){
            if((dict1.count(line) == 0) && (dict2.count(line) == 0)){
              logPHam+=log(1.0/(this->wordsHam+this->wordsSpam));
              logPSpam+=log(1.0/(this->wordsHam+this->wordsSpam));
            }else if((dict1.count(line) == 0) && (dict2.count(line) != 0)){
              logPHam+=log(1.0/(this->wordsHam+this->wordsSpam));
              logPSpam+=log(this->probabilities[line].second);
            }else if((dict1.count(line) != 0) && (dict2.count(line) == 0)){
              logPHam+=log(this->probabilities[line].first);
              logPSpam+=log(1.0/(this->wordsHam+this->wordsSpam));
            }else{
              logPHam+=log(this->probabilities[line].first);
              logPSpam+=log(this->probabilities[line].second);
            }
          }
        }
      }
      if( logPHam > logPSpam ){
        cout<<"ham "<<logPHam<<" "<<logPSpam<<endl;
        this->falsePositive+=1;
      }else{
        cout<<"spam "<<logPHam<<" "<<logPSpam<<endl;
        this->trueNegative+=1;
      }
    }
  }
  fileName.close();
}


int main(int argc, char* argv[]){
  Classifier Spam;
  Classifier Ham;
  Classifier All;
  int smoothing = atoi(argv[argc-1]);
  Classifier Prob;
  for(int i = 1 ; i < 3 ; i++){
    if(i==1){
      Ham.getDictionary(argv[i]);
    }else{
      Spam.getDictionary(argv[i]);
    }
  }
  All.mergeDictionary(Ham.dictionary);
  All.mergeDictionary(Spam.dictionary);
  All.getTotalWords();
  Prob.calculateProbabilities(Ham.dictionary, Spam.dictionary, All.dictionary, smoothing, Ham.totalWords, Spam.totalWords);
  Prob.classProbCalc(smoothing, Ham.totalMessages, Spam.totalMessages);
  Prob.wordsHam = Ham.totalWords;
  Prob.wordsSpam = Spam.totalWords;
  for(int i = 3 ; i < 5 ; i++){
    if(i==3){
      Prob.classifyFileHam(argv[i], Ham.dictionary, Spam.dictionary);
    }else{
      Prob.classifyFileSpam(argv[i], Ham.dictionary, Spam.dictionary);
    }
  }
  double totalSize = (Prob.truePositive+Prob.trueNegative+Prob.falseNegative+Prob.falsePositive);
  double specificity = (Prob.trueNegative/(Prob.trueNegative+Prob.falsePositive));
  double sensitivity = (Prob.truePositive/(Prob.truePositive+Prob.falseNegative));
  double accuracy = ((Prob.truePositive+Prob.trueNegative)/totalSize);
  cout<<specificity<<" "<<sensitivity<<" "<<accuracy<<endl;
  return 0;
}
