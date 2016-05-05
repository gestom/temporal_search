/*
 * File name: graph.cc
 * Date:      2014-08-20 09:33:53 +0200
 * Author:    Miroslav Kulich
 */


#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "graph.h"
#include <algorithm>
#include <limits>
// #include <boost/property_map/property_map.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

#define MAX_PATH_LENGTH 19
#define START_NODE "Center"
#define REMOVE_IMPOSIBLE_NODES false//true //false
#define PRUNING true //false

// STL container iterator macros
#define VAR(V,init) __typeof(init) V=(init)
#define FOR_EACH(I,C) for(VAR(I,(C).begin());I!=(C).end();I++)
#define FOR_EACH_REVERSE(I,C) for(VAR(I,(C).rbegin());I!=(C).rend();I++)

using namespace imr;


/// =============================
/// = Node ======================
/// =============================

/// - constructor --------------------------------------------------------------
Node::Node(int id, std::string name) :
  id(id),
  name(name),
  isPassThrough(false),
  processed(false) {
  };

/// - destructor --------------------------------------------------------------
Node::~Node() {
}

/// - public method --------------------------------------------------------------
void Node::addLink(int id, double weight) {
  links.push_back(Graph::Link(id,weight));
}

/// - public method --------------------------------------------------------------
void Node::setPassThrough(bool val) {
  isPassThrough = val;
}

/// - public method --------------------------------------------------------------
void Node::setProbability(double prob) {
  probability = prob;
}

/// - public method --------------------------------------------------------------
void Node::setAllLinks(int num) {
  allNodes.clear();
  for(int i=0;i<num;i++) {
    if (i!=id) {
      allNodes.push_back(i);
    }
  }
}


/// - public method --------------------------------------------------------------
double Node::getProbability() {
  return probability;
}

struct LinkBetter {
  static Graph &graph;
  bool operator()(Graph::Link const & a, Graph::Link const & b) const  {
    Node &aa = graph.nodes[a.first];
    Node &bb = graph.nodes[b.first];
    if (aa.isPassThrough == bb.isPassThrough ) {
        return a.second * aa.getProbability() > b.second * bb.getProbability();
//        return a.second > b.second;
    } else {
      return bb.isPassThrough;
    }
  }
};

struct NodeBetter {
  static Graph &graph;
  static int node;
  bool operator()(int const & a, int const & b) const  {
      return graph.dist[node][a] < graph.dist[node][b];
  }
};


Graph tmp;
Graph &LinkBetter::graph = tmp;
Graph &NodeBetter::graph = tmp;
int NodeBetter::node;

  /// - public method --------------------------------------------------------------
  void Node::sortLinks() {
    std::sort(links.begin(), links.end(), LinkBetter());
  }

  /// - public method --------------------------------------------------------------
  void Node::sortNodes() {
    NodeBetter::node = id;
    std::sort(allNodes.begin(), allNodes.end(), NodeBetter());
    FOR_EACH(n,allNodes) {
      std::cout << NodeBetter::graph.dist[id][*n] << " ";
    }
    std::cout << std::endl;
  }




/// =============================
/// = Graph ======================
/// =============================


/// - constructor --------------------------------------------------------------
Graph::Graph(std::string fileName) {
  if (!fileName.empty()) {
    load(fileName);
  }
}

/// - destructor --------------------------------------------------------------
Graph::~Graph() {
}

/// - public method --------------------------------------------------------------
int Graph::addNode(std::string name) {
  static int num = 0;
  int result;
  NodeMap::iterator it = nodeMap.find(name);
  if ( it == nodeMap.end() ) {
    nodes.push_back(Node(num,name));
    result = num;
    add_vertex(g);
    nodeMap[name] = num++;
  } else {
    result = it->second;
  }
  return result;
}

/// - public method --------------------------------------------------------------
void Graph::addUndirectedLink(int from, int to, double weight) {
    nodes[to].addLink(from, weight);
    nodes[from].addLink(to, weight);
    add_edge(from,to,weight,g);
}


/// - public method --------------------------------------------------------------
void Graph::display() {
  FOR_EACH(node, nodes) {
    std::cout << node->name << " (" << node->id << ")   " << node->isPassThrough << "   pp: "<< node->probability << std::endl;
    FOR_EACH(link, node->links) {
      std::cout << "   " << link->first << " " << nodes[link->first].name;
      std::cout << " (" << nodes[link->first].isPassThrough << ") ";
      std::cout << " w: " << link->second;
      std::cout << " pp: " << link->second * nodes[link->first].getProbability() << "  " << nodes[link->first].getProbability();
      std::cout << std::endl;
    }
  }

}

/// - public method --------------------------------------------------------------
void Graph::load(std::string fileName) {
  std::ifstream fin;
  std::string type, node, from, to;
  double weight;
  int idFrom, idTo;
  fin.open(fileName.c_str());
  if (!fin.good()) {
    throw std::runtime_error("File " + fileName + " cann't be opened.");
  }

  fin >> type;
  while (!fin.eof())
  {
    if (type.compare("node")==0) {
      fin >> node;
      addNode(node);
    } else if (type.compare("link")==0) {
      fin >> from;
      fin >> to;
      fin >> weight;
      idFrom = addNode(from);
      idTo = addNode(to);
//       std::cout << "adding " << idFrom <<  " " << idTo << " " << weight <<  std::endl;
      addUndirectedLink(idFrom, idTo, weight);
    }
    fin >> type;
  }

  std::vector < double >d(100, (std::numeric_limits < int >::max)());
  boost::johnson_all_pairs_shortest_paths(g, dist, boost::distance_map(&d[0]));

  double minDist;
  int dd;
  for (unsigned int i = 0; i < nodes.size(); ++i) {
    for (unsigned int j = 0; j < nodes.size(); ++j) {
      if (i!=j) {
        minDist = std::numeric_limits <double>::max();
        FOR_EACH(link,nodes[i].links) {
          dd = link->second + dist[link->first][j];
          if (dd < minDist) {
            minDist = dd;
            next[i][j] = link->first;
          }
        }
      } else {
        next[i][j] = i;
      }
    }
  }


  std::cout << "Distances" << std::endl << "       ";
  for (unsigned int k = 0; k < nodes.size(); ++k) {
    std::cout << std::setw(5) << k;
  }
  std::cout << std::endl;

  for (unsigned int i = 0; i < nodes.size(); ++i) {
    std::cout << std::setw(3) << i << " -> ";
    for (unsigned int j = 0; j < nodes.size(); ++j) {
      if (dist[i][j] == (std::numeric_limits<int>::max)())
        std::cout << std::setw(5) << "inf";
      else
        std::cout << std::setw(5) << dist[i][j];
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Nexts"<< std::endl << "       ";
  for (unsigned int k = 0; k < nodes.size(); ++k) {
    std::cout << std::setw(5) << k;
  }
  std::cout << std::endl;

  for (unsigned int i = 0; i < nodes.size(); ++i) {
    std::cout << std::setw(3) << i << " -> ";
    for (unsigned int j = 0; j < nodes.size(); ++j) {
      if (dist[i][j] == (std::numeric_limits<int>::max)())
        std::cout << std::setw(5) << "inf";
      else
        std::cout << std::setw(5) << next[i][j];
    }
    std::cout << std::endl;
  }



  // print boost graph structure

  std::ofstream fout("fig.dot");
  fout << "digraph A {\n"
  << "  rankdir=LR\n"
  << "size=\"5,3\"\n"
  << "ratio=\"fill\"\n"
  << "edge[style=\"bold\"]\n" << "node[shape=\"circle\"]\n";

  boost::graph_traits < BoostGraph >::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei)
//    fout << nodes[boost::source(*ei, g)].name << " -> " << nodes[boost::target(*ei, g)].name
    fout << boost::source(*ei, g) << " -> " << boost::target(*ei, g)
    << "[label=" << boost::get(boost::edge_weight, g)[*ei] << "]\n";

  fout << "}\n";

}


/// - public method --------------------------------------------------------------
void Graph::setProbabilities(Probabilities probs) {
  NodeBetter::graph = *this;
  for(unsigned int i=0;i<probs.size();i++) {
      nodes[i].setProbability(probs[i]);
  }
  labelPassThroughPlaces();
  sortLinks();
  FOR_EACH(node,nodes) {
    node->setAllLinks(probs.size());
    node->sortNodes();
  }
}

/// - public method --------------------------------------------------------------
void Graph::sortLinks() {
  LinkBetter::graph = *this;
  FOR_EACH(node,nodes) {
    node->sortLinks();
  }
}

/// - public method --------------------------------------------------------------
void Graph::labelPassThroughPlaces() {
  FOR_EACH(node,nodes) {
    node->isPassThrough = node->links.size() > 1;
  }
}


/// - public method --------------------------------------------------------------
double Graph::estimateRest(double pathLength) {
  if (PRUNING) {
    double result = 0;
    std::vector<double> length;
    std::vector<double> probability;

    FOR_EACH(node, nodes) {
      if (!node->processed) {
        length.push_back(node->allNodes[0]);
        probability.push_back(node->probability);
      }
    }
    std::sort(length.begin(),length.end());
    std::sort(probability.begin(),probability.end());
    double pl = pathLength;
    for(unsigned int i=0;i<length.size();i++) {
      pl += length[i];
      result += pl * probability[i];
    }
    return result;
  } else return 0;
}

/*
/// - public method --------------------------------------------------------------
double Graph::estimateRest(double pathLength) {
  if (PRUNING) {
    double result = 0;
    FOR_EACH(node, nodes) {
      if (!node->processed) {
        result += (pathLength+node->links[0].second) * node->probability;
      }
    }
    return result;
  } else return 0;
}
*/

int perNum;
/// - public method --------------------------------------------------------------
void Graph::permuteNodes(int id, int size) {
//   int numNodes = nodes.size();
	if ( pathSize == numPossibleNodes || (pathCost + estimateRest(pathLength) > best) ) {
		if (false) {
			std::cout  << "[" << perNum << "]" << pathSize << " " << pathCost << " " << pathLength << ":  ";
			for(unsigned int i=0;i<path.size();i++) {
				std::cout << path[i] << " ";
			}
			std::cout  << std::endl;
		}
		if ( (pathCost < best || (pathCost == best && path.size() < bestPath.size() ) ) && pathSize == numPossibleNodes) {
			best = pathCost;
			bestPermNumber = perNum;
			bestPath.resize(path.size(),-1);
			std::copy(path.begin(),path.end(),bestPath.begin());
			bestPathTimes.resize(path.size(),-1);
			std::copy(pathTimes.begin(),pathTimes.end(),bestPathTimes.begin());
			if (false) {
				std::cout  << pathCost << " " << pathLength << ":  ";
				for(unsigned int i=0;i<bestPath.size();i++) {
					std::cout << bestPath[i] << " (" << bestPathTimes[i] << ") ";
				}
				std::cout  << std::endl;
			}
		}
		perNum++;
	} else {
		FOR_EACH(n,nodes[id].allNodes) {
			Node &neib = nodes[*n];
			if (!neib.processed) {
				std::vector<int> added;
				int act = id;
				unsigned int ps = path.size();
				int prev;
				double pCost = pathCost;
				int pSize = pathSize;
				//         std::cout << " last: " << *n << " ";
				do {
					prev = act;
					act = next[act][*n];
					pathLength += dist[prev][act];
					pathTimes.push_back(pathLength);
				//	           std::cout << " adding " << prev << " -> " << act << " " << path.size()+1 << std::endl;
					if ( !nodes[act].processed ) {
						added.push_back(act);
						nodes[act].processed = true;
						pathCost += pathLength*nodes[act].probability;
						pathSize++;
					}
					path.push_back(act);
				} while  ( act != (*n) );
				 //        std::cout << "Path size " << pathSize << std::endl;
				 //        std::cout << "dist " << id << " " << *n << " " << dist[id][*n] << std::endl;

				permuteNodes(*n,size-1);
				pathLength -= dist[id][*n];
				FOR_EACH(a,added) {
					nodes[*a].processed = false;
				}
				while (path.size() > ps) {
					path.pop_back();
					pathTimes.pop_back();
				}
				pathCost = pCost;
				pathSize = pSize;
			}
		}
	}
}


/// - public method --------------------------------------------------------------
Graph::PathSolution Graph::getPath() {
  PathSolution result;
  numPossibleNodes = 1;
  FOR_EACH(node,nodes) {
    if (REMOVE_IMPOSIBLE_NODES) {
      node->processed = node->probability == 0;
      if (!node->processed) {
        numPossibleNodes++;
      }
    } else {
      node->processed = false;
    }
  }
  if (!REMOVE_IMPOSIBLE_NODES) {
    numPossibleNodes = nodes.size();
  }

  std::cout << "numPossibleNodes "  << numPossibleNodes << std::endl;
  path.clear();
  pathTimes.clear();
//   int start = nodeMap["Corridor"];
  int start = nodeMap[START_NODE];
  path.push_back(start);
  pathTimes.push_back(0);
  nodes[start].processed = true;
  printf("START %i\n",start);
  pathLength = 0;
  pathCost = 0;
  pathSize = 1;
  best = std::numeric_limits<double>::max();
  perNum = 0;
  permuteNodes(start,MAX_PATH_LENGTH);
  result.path.resize(bestPath.size());
  std::copy(bestPath.begin(), bestPath.end(), result.path.begin());
  result.pathTimes.resize(bestPath.size());
  std::copy(bestPathTimes.begin(), bestPathTimes.end(), result.pathTimes.begin());
  result.expectedTime = best;
  return result;
}

struct NN {
  int id;
  bool done;
  NN(int id, bool done) : id(id), done(done) {};
  NN() {};
};

/// - public method --------------------------------------------------------------
Graph::PathSolution Graph::getPath2() {
  PathSolution result;
  /*
   *  numPossibleNodes = 1;
   *  FOR_EACH(node,nodes) {
   *    if (REMOVE_IMPOSIBLE_NODES) {
   *      node->processed = node->probability == 0;
   *      if (!node->processed) {
   *        numPossibleNodes++;
}
} else {
  node->processed = false;
}
}
if (!REMOVE_IMPOSIBLE_NODES) {
  numPossibleNodes = nodes.size();
}
std::cout << "numPossibleNodes "  << numPossibleNodes << std::endl;
path.clear();
pathTimes.clear();
int start = nodeMap[START_NODE];
path.push_back(start);
pathTimes.push_back(0);
nodes[start].processed = true;
pathLength = 0;
pathCost = 0;
pathSize = 1;
best = std::numeric_limits<double>::max();
perNum = 0;
*/

  std::vector<NN> nodes;
  std::stack<int> stack;
  std::vector<int> path;
  for(int i=0;i<5;i++) {
    nodes.push_back(NN(i,false));
    stack.push(i);
  }



  int n;
  while (!stack.empty()) {
    n = stack.top();
    stack.pop();
//    std::cout << n << " ";

    path.push_back(n);
    nodes[n].done = true;
    if (path.size() == 5) {
      FOR_EACH(nn,path) {
        std::cout << *nn << " ";
      }
      std::cout << std::endl;
      nodes[n].done = false;
      path.pop_back();
    } else {
      FOR_EACH(neib,nodes) {
        if (!neib->done) {
          stack.push(neib->id);
        }
      }
    }
  }

    /*
     *  result.path.resize(bestPath.size());
     *  std::copy(bestPath.begin(), bestPath.end(), result.path.begin());
     *  result.pathTimes.resize(bestPath.size());
     *  std::copy(bestPathTimes.begin(), bestPathTimes.end(), result.pathTimes.begin());
     *  result.expectedTime = best;
     */
    return result;
  }




/// - public method --------------------------------------------------------------
void Graph::displayPath(Path path, Path times, bool symbolic) {
  if (path.empty()) return;
  std::cout << "[" << path.size() << "] ";
  if (symbolic) {
    std::cout << nodes[path[0]].name;
    for(unsigned int i=1;i<path.size();i++) {
      std::cout << "->" << nodes[path[i]].name << " (" << times[i] << ") ";
    }
  } else {

    for(unsigned int i=0;i<path.size();i++) {
      std::cout << nodes[path[i]].id << " (" << times[i] << ") ";
    }
  }
  std::cout  << std::endl;
}

/* end of graph.cc */
