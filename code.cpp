#include<iostream>
#include<fstream>
#include<vector>
#include<set>
#include<map>
#include<cassert>
#include<string>
#include<cmath>
#include<ctime>
using namespace std;
#define ld long double
#define ll long long
//baseNumNode used for file addressing
int baseNumNode = 0;
int baseDataFile = 0;
string makeNewNodeName()
{
	//Return a new file address for the node
	string ans = "AAAAAA";
	int curr = baseNumNode;
	baseNumNode++;
	for(int i=5;i>=0;i--)
	{
		ans[i] += curr % 26;
		curr /= 26;
	}
	return "nodes/" + ans  + ".txt";
}
string makeNewDataFileName()
{
	//Return a new file address for the node
	string ans = "AAAAAA";
	int curr = baseDataFile;
	baseDataFile++;
	for(int i=5;i>=0;i--)
	{
		ans[i] += curr % 26;
		curr /= 26;
	}
	return "data/" + ans + ".txt";
}
int maxKeys = 0;
bool rebuild = 0;
bool error = 0;
ll cnt=0;
string rootName;
void readConfig()
{
	//Read maxKey from predefined file
	string fileName = "bplustree.config";
	ifstream fin(fileName.c_str(), ios::in);
	fin>>maxKeys;
	fin>>rebuild;
	fin.close();
	if(maxKeys < 3)
	{
		cout<<"Max Keys should be atleast 3\n";
		error = 1;
		return;
	}
	if(rebuild)
	{
		cout<<"Building tree as asked\n";
		return;
	}
	fileName = ".builtTreeConfig";
	if (FILE *file = fopen(fileName.c_str(), "r")) 
	{
		fclose(file);
		//Old tree exists
		ifstream fin(fileName.c_str(), ios::in);
		int mk;
		fin>>rootName;
		fin>>mk;
		fin>>baseNumNode;
		fin>>baseDataFile;
		fin.close();
		if(mk != maxKeys)
		{
			cout<<"Max Keys changed! Rebuilding!\n";
			baseNumNode = 0;
			baseDataFile = 0;
			rebuild = 1;
			return;
		}
	} 
	else 
	{
		//Old tree doesn't exist
		cout<<"Old tree not found. Rebuilding!\n";
		rebuild = 1;
		return;
	} 
}
void writeConfigData()
{
	//Write rootName so that it can be read on later executions of code
	string fileName = ".builtTreeConfig";
	ofstream fout(fileName.c_str(), ios::out);
	fout<<rootName<<endl;
	fout<<maxKeys<<endl;
	fout<<baseNumNode<<endl;
	fout<<baseDataFile<<endl;
	fout.close();
}
///////////////////
//Statistics Info//
///////////////////
vector<long long> itime,ptime,rtime;
vector<long long> ifile,pfile,rfile;
ld avg(vector<ll> x)
{
	if((int)x.size()==0)return 0;
	ld ans=0;
	for(int i=0;i<(int)x.size();++i)ans+=x[i];
	return ans/(int)x.size();
}
long long mmax(vector<ll> x)
{
	long long ans = 0;
	for(int i=0;i<(int)x.size();++i)ans=max(ans,x[i]);
	return ans;
}
long long mmin(vector<ll> x)
{
	if((int)x.size() == 0)return 0;
	long long ans = x[0];
	for(int i=1;i<(int)x.size();++i)ans=min(ans,x[i]);
	return ans;
}
ld stdev(vector<ll> x)
{
	if((int)x.size()==0)return 0;
	ld a = avg(x);
	ld ans=0;
	for(int i=0;i<(int)x.size();++i)
	{
		ans+=(x[i]-a)*(x[i]-a);
	}
	ans/=(int)x.size();
	return sqrt(ans);
}
//////////////////
//B+ Tree Starts//
//////////////////
//Node for B+ tree
struct node
{
	string fName;	//Filename (addressed by 6 characters)
	bool isRoot;	//To denote whether this node is a root or not
	bool isLeaf;	//To denote whether a this is a leaf or not
	int numKeys;	//number of keys in this node
	string parent;	//pointer to parent file
	vector<ld> keys;	//keys stored
	vector<string> children;	//Children pointers
};
//Write this node to its file
void writeNode(node x)
{
	cnt++;
	ofstream fout(x.fName.c_str(), ios::out);
	fout<<x.isRoot<<endl;
	fout<<x.isLeaf<<endl;
	fout<<x.numKeys<<endl;
	fout<<x.parent<<endl;
	for(int i=0;i<x.numKeys;++i)
		fout<<x.keys[i]<<" ";
	fout<<endl;
	for(int i=0;i<=x.numKeys;++i)
		fout<<x.children[i]<<" ";
	fout.close();
}
//Write the data in a file
void writeData(string fName, string data)
{
	cnt++;
	ofstream fout(fName.c_str(),ios::out);
	fout<<data;
	fout.close();
}
//Read a node from a file
node readNode(string fName)
{
	cnt++;
	node ans;
	ans.fName = fName;
	ifstream fin(fName.c_str(), ios::in);
	fin>>ans.isRoot;
	fin>>ans.isLeaf;
	fin>>ans.numKeys;
	fin>>ans.parent;
	ans.keys.resize(ans.numKeys);
	ans.children.resize(ans.numKeys+1);
	for(int i=0;i<ans.numKeys;++i)fin>>ans.keys[i];
	for(int i=0;i<=ans.numKeys;++i)fin>>ans.children[i];
	fin.close();
	return ans;
}
//Read the data from a file
string readData(string fName)
{
	cnt++;
	string data;
	ifstream fin(fName.c_str(), ios::in);
	fin>>data;
	fin.close();
	return data;
}
//Make a new node
//This might be a leaf if making the root (first time)
node makeNewNode(bool isLeaf = false)
{
	//Note that a new node is always a root
	node now;
	now.fName = makeNewNodeName();
	now.isRoot = 1;
	now.isLeaf = isLeaf;
	now.parent = "NULL";
	now.numKeys = 0;
	now.keys.clear();
	now.children.clear();
	now.children.push_back("NULL");
	return now;
}
//Init an empty BTree
void init()
{
	node root = makeNewNode(true);
	rootName = root.fName;
	writeNode(root);
	writeConfigData();
	return;
}
//Find the leaf node corresponding to a key
node findLeaf(ld key, string nodePtr = rootName)
{
	assert(nodePtr != "NULL");
	node now = readNode(nodePtr); 
	if( now.isLeaf ) return now;
	//Now find the first element greater or equal than key
	int idx = lower_bound(now.keys.begin(),now.keys.end(),key-1.0e-10) - now.keys.begin();	//epsilon used for floating point error
	return findLeaf(key, now.children[idx]); 
}
//Add key to an internal node
//left, right are the pointers
//Returns the pointer to the parents of left and right
pair<string,string> addValueToNode(ld key, string nowPtr, string left, string right)
{
	if( nowPtr == "NULL" )
	{
		//Need to make a new root
		node now = makeNewNode();
		now.numKeys++;
		now.keys.push_back(key);
		now.children.push_back(right);
		now.children[0] = left;
		writeNode(now);
		rootName = now.fName;
		return make_pair(now.fName,now.fName);
	}
	else
	{
		//Add in current node
		node now = readNode(nowPtr);
		//Now the left pointer is always present in now
		//Search for it and insert key and right pointer in the next place
		int idx = 0;
		while(idx <= now.numKeys && now.children[idx] != left)
			idx++;
		assert(idx <= now.numKeys);
		//Extend the vectors
		now.numKeys++;
		now.keys.push_back(key);
		now.children.push_back(right);
		//Insert it in the right place
		for(int i = now.keys.size() - 2;i>=idx;i--)
			now.keys[i+1]=now.keys[i];
		for(int i = now.children.size() - 2;i>=idx+1;i--)
			now.children[i+1] = now.children[i];
		now.keys[idx] = key;
		now.children[idx+1] = right;
		//Check if it exceeds capacity
		if(now.numKeys > maxKeys)
		{
			//Make a new node
			node rightNode = makeNewNode();
			//Now set the return parents correctly
			pair<string, string> ret;
			if(idx < now.numKeys/2 )
			{
				//Added Key is on left
				ret = make_pair(now.fName, now.fName);
			}
			else if(idx > now.numKeys/2)
			{
				//Added key is on right
				ret = make_pair(rightNode.fName, rightNode.fName);
			}
			else
			{
				//Added key is propogated upwards
				ret = make_pair(now.fName, rightNode.fName);
			}
			//Propogate the split upwards
			int n = now.numKeys;
			idx = now.numKeys/2;	//Index to remove
			vector<ld> keys = now.keys;
			vector<string> children = now.children;
			//Clear Now
			now.numKeys = 0;
			now.children.clear();
			now.keys.clear();
			now.isRoot = 0;
			rightNode.isRoot = 0;
			rightNode.children.clear();
			//Now distribute the keys
			now.numKeys = idx;
			now.keys.resize(now.numKeys);
			now.children.resize(now.numKeys+1);
			for(int i=0;i<idx;++i)
			{
				now.keys[i] = keys[i];
				now.children[i] = children[i];
			}
			now.children[now.numKeys] = children[idx];
			//Make the new right node
			rightNode.numKeys = n-idx-1;
			rightNode.keys.resize(rightNode.numKeys);
			rightNode.children.resize(rightNode.numKeys+1);
			for(int i=idx+1;i<n;++i)
			{
				rightNode.keys[i-idx-1] = keys[i];
				rightNode.children[i-idx-1] = children[i];
			}
			rightNode.children[rightNode.numKeys] = children[n];
			//Need to patch all the children parents! 
			//All the children point to now. Need to make the children of rightNode point to rightNode
			//This is computationally heavy
			//One way is to patch the parents in findNode. This results in correct exec, but inconsistent data
			for(int i = 0;i<rightNode.children.size();++i)
			{
				node child = readNode(rightNode.children[i]);
				child.parent = rightNode.fName;
				writeNode(child);
			}
			//Now push the key above
			pair<string, string> parents = addValueToNode(keys[idx], now.parent, now.fName, rightNode.fName);
			now.parent = parents.first;
			rightNode.parent = parents.second;
			writeNode(now);
			writeNode(rightNode);
			return ret;
		}
		else
		{
			//Peacefully Write
			writeNode(now);
			return make_pair(nowPtr,nowPtr);
		}
	}
}
//Insert into the B+Tree
//Find the leaf and split if needed
//Propogate till root if needed
void insert(ld key, string data)
{
	node now = findLeaf(key);
	int idx = lower_bound(now.keys.begin(),now.keys.end(),key - 1.0e-10) - now.keys.begin();
	string newFile = makeNewDataFileName();
	writeData(newFile, data);
	//Add entry in now
	now.numKeys++;
	now.keys.push_back(key);
	now.children.push_back(newFile);
	//Push all the elements 1 place right from idx onwards
	for(int i = now.keys.size() - 2;i>=idx;i--)
		now.keys[i+1]=now.keys[i];
	for(int i = now.children.size() - 2;i>=idx;i--)
		now.children[i+1] = now.children[i];
	now.keys[idx] = key;
	now.children[idx] = newFile;
	//Check if capacity increased!
	if( now.numKeys > maxKeys )
	{
		//Need to split
		node rightNode = makeNewNode(true);
		rightNode.isRoot = 0;
		rightNode.children.clear();
		int n = now.numKeys;
		vector<ld> keys = now.keys;
		vector<string> children = now.children;
		now.numKeys = 0;
		now.keys.clear();
		now.children.clear();
		now.isRoot = 0;
		//Now distribute keys in two nodes equally (now and rightNode)
		//Make now
		now.numKeys = n/2;
		now.keys.resize(now.numKeys);
		now.children.resize(now.numKeys+1);
		for(int i=0;i<now.numKeys;++i)
		{
			now.keys[i] = keys[i];
			now.children[i] = children[i];
		}
		//The last pointer in now is to rightNode
		now.children[now.numKeys] = rightNode.fName;
		//Rest of the keys and children are in rightNode
		rightNode.numKeys = n - now.numKeys;
		rightNode.keys.resize(rightNode.numKeys);
		rightNode.children.resize(rightNode.numKeys+1);
		for(int i=now.numKeys;i<n;++i)
		{
			rightNode.keys[i-now.numKeys] = keys[i];
			rightNode.children[i-now.numKeys] = children[i];
		}
		rightNode.children[rightNode.numKeys] = children[n];
		//Now push the last key of now above
		pair<string, string> parents = addValueToNode(now.keys[now.numKeys-1],now.parent, now.fName, rightNode.fName);
		now.parent = parents.first;
		rightNode.parent = parents.second;
		writeNode(now);
		writeNode(rightNode);
	}
	else
	{
		//Peacefully Insert
		writeNode(now);
	}
	writeConfigData();
}
void pquery(ld key)
{
	cout<<"Point Query (" <<key<<"): ";
	node now = findLeaf(key);
	bool found = 0;
	for(int i=0;i<=now.keys.size();++i)
	{
		assert(now.isLeaf);
		if(i == now.keys.size())
		{
			//Goto next node
			if(now.children[i] == "NULL")break;
			now = readNode(now.children[i]);
			i=-1;
		}
		if(now.keys[i] > key - 1.0e-10 && now.keys[i] < key + 1.0e-10)
		{
			//If this value is key, then display
			if(found)cout<<", ";
			found = 1;
			string data = readData(now.children[i]);
			cout<<data;
		}
		if(now.keys[i] > key + 1.0e-10)
			break;
	}
	if(!found)
		cout<<"Key does not exist!";
	cout<<endl;
}
void rquery(ld c, ld r)
{
	cout<<"Range Query (c:"<<c<<",r:"<<r<<"): ";
	ld key1 = c-r, key2 = c+r;
	node now = findLeaf(key1);
	bool found = 0;
	for(int i=0;i<=now.keys.size();++i)
	{
		assert(now.isLeaf);
		if(i == now.keys.size())
		{
			//Goto next node
			if(now.children[i] == "NULL")break;
			now = readNode(now.children[i]);
			i=-1;
		}
		if(now.keys[i] > key1 - 1.0e-10 && now.keys[i] < key2 + 1.0e-10)
		{
			//If this value is key, then display
			if(found)cout<<", ";
			found = 1;
			string data = readData(now.children[i]);
			cout<<now.keys[i]<<":"<<data;
		}
		if(now.keys[i] > key2 + 1.0e-10)
			break;
	}
	if(!found)
		cout<<"No Keys found!";
	cout<<endl;
}
int main()
{
	clock_t t;
	readConfig();
	if(error)
	{
		cout<<"Aborting! See you next time!\n";
		return 0;
	}
	if(rebuild)
	{
		//Build the initial tree
		init();
		//Read initial points
		string fileName = "assgn2_bplus_data.txt";
		ifstream fin(fileName.c_str(), ios::in);
		while(!fin.eof())
		{
			//Insert these elements
			ld key;
			fin>>key;
			if(fin.eof())break;
			string data;
			fin>>data;
			insert(key, data);
		}
		fin.close();
		cout<<"Initial Tree Built\n";
	}
	cout<<"Enter Queries: \n";
	while(!feof(stdin))
	{
		int type;
		cin>>type;
		if(feof(stdin))break;
		if(type < 0 || type > 2)break;
		if(type == 0)
		{
			//Insert Query
			ld key;
			string data;
			cin>>key>>data;
			cnt = 0;
			t = clock();
			insert(key, data);
			t = clock() - t;
			itime.push_back((1000000*t)/CLOCKS_PER_SEC);
			ifile.push_back(cnt);
			cout<<"Insert ("<<key<<":"<<data<<"): Success\n";
		}
		else if(type == 1)
		{
			//Point query
			ld key;
			cin>>key;
			cnt = 0;
			t = clock();
			pquery(key);
			t = clock() - t;
			ptime.push_back((1000000*t)/CLOCKS_PER_SEC);
			pfile.push_back(cnt);

		}
		else
		{
			//Range Query
			ld key1,key2;
			cin>>key1>>key2;
			cnt = 0;
			t = clock();
			rquery(key1, key2);
			t = clock() - t;
			rtime.push_back((1000000*t)/CLOCKS_PER_SEC);
			rfile.push_back(cnt);
		}
	}
	cout<<"############\n";
	cout<<"#STATISTICS#\n";
	cout<<"############\n";
	//Insert Queries
	cout<<"\n#Insert Queries: \n";
	cout<<"Number of Queries: "<<itime.size()<<"\n";
	cout<<"Average: time:"<<avg(itime)<<" (us), files accessed:"<<avg(ifile)<<endl;
	cout<<"Maximum: time:"<<mmax(itime)<<" (us), files accessed:"<<mmax(ifile)<<endl;
	cout<<"Minimum: time:"<<mmin(itime)<<" (us), files accessed:"<<mmin(ifile)<<endl;
	cout<<"Standard Deviation: time:"<<stdev(itime)<<" (us), files accessed:"<<stdev(ifile)<<endl;
	//Point Queries
	cout<<"\n#Point Queries: \n";
	cout<<"Number of Queries: "<<ptime.size()<<"\n";
	cout<<"Average: time:"<<avg(ptime)<<" (us), files accessed:"<<avg(pfile)<<endl;
	cout<<"Maximum: time:"<<mmax(ptime)<<" (us), files accessed:"<<mmax(pfile)<<endl;
	cout<<"Minimum: time:"<<mmin(ptime)<<" (us), files accessed:"<<mmin(pfile)<<endl;
	cout<<"Standard Deviation: time:"<<stdev(ptime)<<" (us), files accessed:"<<stdev(pfile)<<endl;
	//Range Queries
	cout<<"\n#Range Queries: \n";
	cout<<"Number of Queries: "<<rtime.size()<<"\n";
	cout<<"Average: time:"<<avg(rtime)<<" (us), files accessed:"<<avg(rfile)<<endl;
	cout<<"Maximum: time:"<<mmax(rtime)<<" (us), files accessed:"<<mmax(rfile)<<endl;
	cout<<"Minimum: time:"<<mmin(rtime)<<" (us), files accessed:"<<mmin(rfile)<<endl;
	cout<<"Standard Deviation: time:"<<stdev(rtime)<<" (us), files accessed:"<<stdev(rfile)<<endl;
	cout<<endl;
	return 0;
}
