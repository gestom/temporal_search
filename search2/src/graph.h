/*
 * File name: graph.h
 * Date:      2014-08-20 09:33:53 +0200
 * Author:    Miroslav Kulich
 */

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <string>
#include <vector>
#include <map>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/adjacency_list.hpp>

#define MAX_GRAPH_SIZE 100
namespace imr {

/// class
class Node;
class Graph {
public:
  typedef std::vector<Node> Nodes;
  typedef std::vector<double> Probabilities;
  typedef std::map<std::string,int> NodeMap;
  typedef std::pair<int,double> Link;
  typedef std::vector<int> Path;
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property,
  boost::property< boost::edge_weight_t, double > > BoostGraph;
//  boost::property< boost::edge_weight_t, double, boost::property< boost::edge_weight2_t, double > > > BoostGraph;
  struct PathSolution {
    Path path;
    double expectedTime;
    Path pathTimes;
  };

  Nodes nodes;
  NodeMap nodeMap;
  Path path, bestPath, pathTimes, bestPathTimes;
  int bestPermNumber;
  double pathLength;
  double pathCost;
  double best;
  int pathSize;
  int numPossibleNodes;
  BoostGraph g;
  double dist[MAX_GRAPH_SIZE][MAX_GRAPH_SIZE];
  int next[MAX_GRAPH_SIZE][MAX_GRAPH_SIZE];


  int addNode(const std::string name);
  void addUndirectedLink(int from, int to, double weight);
  void setProbabilities(Probabilities probs);

  void load(const std::string fileName);
  void display();
  void sortLinks();
  void labelPassThroughPlaces();
  void permuteNodes(int id, int size);
  PathSolution getPath();
  PathSolution getPath2();
  void displayPath(Path path,Path times,bool symbolic=true);
  double estimateRest(double pathLength);
  Graph(const std::string fileName="");
  ~Graph();
};

/// class
class Node {
public:
  int id;
  std::string name;
  double probability;
  bool isPassThrough;
  bool processed;
  std::vector<Graph::Link> links;
  std::vector<int> allNodes;
  Node(int id, std::string name);
  void addLink(int id, double weight);
  void setPassThrough(bool val);
  void setProbability(double prob);
  double getProbability();
  void sortLinks();
  void sortNodes();
  void setAllLinks(int num);
  ~Node();
};

}

#endif

/* end of graph.h */
