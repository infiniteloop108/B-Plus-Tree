#include<iostream>
#include<fstream>
#include<vector>
#include<set>
#include<map>
#include<cassert>
#include<string>
using namespace std;
#define ld long double
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
//Max keys in a node
int maxKeys = 0;
void readMaxKeys()
{
	//Read maxKey from predefined file
	string fileName = "bplustree.config";
	ifstream fin(fileName.c_str(), ios::in);
	fin>>maxKeys;
	fin.close();
}
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
	ofstream fout(fName.c_str(),ios::out);
	fout<<data;
	fout.close();
}
//Read a node from a file
node readNode(string fName)
{
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
//root node
string rootName;
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
	return;
}
//Find the leaf node corresponding to a key
node findLeaf(ld key, string nodePtr = rootName)
{
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
			//Normally the return value will be nowPtr, nowPtr if the key just added isn't pushed above
			pair<string, string> ret = make_pair(nowPtr, nowPtr);
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
			node rightNode = makeNewNode();
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
			now.children[now.numKeys] = children[idx+1];
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
			//If the just added key is removed, then the parents of left and right will be different
			if(children[idx] == left && children[idx+1]==right)
				ret = make_pair(nowPtr, rightNode.fName);
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
}
int main()
{
	readMaxKeys();
	//Build the initial tree
	init();
	//Read initial points
	string fileName = "assgn2_bplus_data.txt";
	ifstream fin(fileName.c_str(), ios::in);
	int pt=0;
	while(!fin.eof())
	{
		if(pt == 7)break;
		//Insert these elements
		ld key;
		fin>>key;
		if(fin.eof())break;
		string data;
		fin>>data;
		insert(key, data);
		cerr<<++pt<<endl;
	}
	fin.close();
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
			insert(key, data);
		}
		else if(type == 1)
		{
			//Point query
			ld key;
			cin>>key;
		}
		else
		{
			//Range Query
			ld key1,key2;
			cin>>key1>>key2;
		}
	}
	return 0;
}
