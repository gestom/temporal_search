## Temporal search (Where is Waldo at time t?)

This repository contains algorightms and data that we used for our work on robotic search that was presented at ICRA 2015 [[1](#references)]. The software we provide reproduces experiments performed on the dataset described in Section 6A of [[1](#references)].

The software provided consists of two packages: 
1. The <i>graph search library</i>, which implements the algorithm described in Section 5 of [[1](#references)].
2. The  <i>temporal modeling and evaluation utilities</i>, which performs the actual experiments.

## How to compile and run it

The project uses standard libraries and we provide relevant Makefiles to compile it.

### Graph search library

Simply go to the <i>search2</i> folder and call <i>make'</i>. 
If successful, the graph-search library  <b>libsearch.a</b> and a testing util <b>search2</b> should appear in the current folder. You can try to test the search lirbary simply by calling <i>./search2</i>. If successful, you should see a the search plan description, which should look like:

Best path (exp. time = 41.1147): 
Symbolic: [18] Center->Master_Bedroom (15) ->Center (30) ->Living_Room (43) ->Kitchen (57) ->Center (71) ->Corridor (92) ->Second_Bedroom (117) ->Corridor (142) ->Office (166) ->Corridor (190) ->Center (211) ->Master_Bedroom (226) ->Master_Bathroom (245) ->Master_Bedroom (264) ->Center (279) ->Corridor (300) ->Second_Bathroom (326) 

###References

1. 6. T.Krajnik, M.Kulich, L.Mudrova, R.Ambrus, T.Duckett: <b>[Where is waldo at time t? using spatio-temporal models for mobile robot search.](http://raw.githubusercontent.com/wiki/gestom/fremen/papers/fremen_2015_ICRA_search.pdf)</b> In proceedings of the IEEE International Conference on Robotics and Automation (ICRA), 2015. [[bibtex](http://raw.githubusercontent.com/wiki/gestom/fremen/papers/fremen_2015_ICRA_search.bib)]
