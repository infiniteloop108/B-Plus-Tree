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
	ifstream fin(fileName, ios::in);
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
	ofstream fout(x.fName, ios::out);
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
	ofstream fout(fName,ios::out);
	fout<<data;
	fout.close();
}
//Read a node from a file
node readNode(string fName)
{
	node ans;
	ans.fName = fName;
	ifstream fin(fName, ios::in);
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
	int idx = lower_bound(now.keys.begin(),now.keys.end(),idx-1.0e-10) - now.keys.begin();	//epsilon used for floating point error
	return findLeaf(key, now.children[idx]); 
}
//Add key to an internal node
//left, right are the pointers
//Returns the pointer to the node in which the key was inserted
string addValueToNode(ld key, string nowPtr, string left, string right)
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
		return now.fName;
	}
	else
	{

		return nowPtr;
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
		//now.parent = addValueToNode(now.keys[now.numKeys-1],now.parent, now.fName, rightNode.fName);
		rightNode.parent = now.parent;
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
	ifstream fin(fileName, ios::in);
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
			//insert(key, data);
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
