/*
 * File name: main.cc
 * Date:      2013-10-31 12:14:25 +0100
 * Author:    Miroslav Kulich
 */

#include <iostream>


#include "graph.h"



int main(int argc, char* argv[]) {
  try {
//    double d[] = {0.3699, 0.0284, 0.3330, 0.0554, 0.0126, 0.0528, 0.0214, 0.0019, 0.1248};
    double d[] = {0.3699, 0.0284, 0.3330, 0.0554, 0.0041, 0.0085, 0.0528, 0.0214, 0.0019, 0.1248 };

//    double d[] = {0.3699, 0.0284, 0.3330, 0.0554, 0.0126, 0.0528, 0.0214, 0.0019, 0.1248, 0.2073, 0.2976, 0.0119};

//    double d[] = {0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.2867, 0.0387, 0.4395, 0.0119, 0.2073, 0.2976};//, 0.1248};
//    double d[] = {0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000};//, 0.1248};
    imr::Graph::Probabilities probs (d,d+9);//+11

//   imr::Graph graph("etc/withamwharf.txt");
    imr::Graph graph("etc/aruba.txt");
    graph.setProbabilities(probs);
    graph.display();
    imr::Graph::PathSolution solution = graph.getPath();
    std::cout << "Best path (exp. time = " << solution.expectedTime << "): " << std::endl;
    std::cout << "Symbolic: ";
    graph.displayPath(solution.path,solution.pathTimes, true);
    std::cout << "IDs: ";
    graph.displayPath(solution.path, solution.pathTimes, false);
    return 0;
  } catch (std::exception &e) {
    std::cout<<"Caught exception: " <<e.what()<< std::endl;
    return -1;
  }
}


/* end of main.cc */
