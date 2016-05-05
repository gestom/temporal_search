## Temporal search (Where is Waldo at time t?)

This repository contains algorightms and data that we used for our work on robotic search that was presented at ICRA 2015 [[1](#references)]. The software we provide reproduces experiments performed on the dataset described in Section 6A of [[1](#references)].

The software provided consists of two packages: 
1. The <i>graph search library</i>, which implements the algorithm described in Section 5 of [[1](#references)].
2. The  <i>temporal modeling and evaluation utilities</i>, which performs the actual experiments.

## How to compile and run it

The project uses standard libraries and we provide relevant Makefiles to compile it.

### Graph search library

Simply `cd search2` to the search2 folder and call `make`. 
If successful, the graph-search library  <b>libsearch.a</b> and a testing util <b>search</b> should appear in the current folder. You can try to test the search lirbary simply by calling <i>./search2</i>. If successful, you should see a the search plan description, which should look like:

```
Best path (exp. time = 41.1147):
Symbolic: [18] Center->Master_Bedroom (15) ->Center (30) ->Living_Room (43) ->Kitchen (57) ->Center (71) ->Corridor (92) ->Second_Bedroom (117) ->Corridor (142) ->Office (166) ->Corridor (190) ->Center (211) ->Master_Bedroom (226) ->Master_Bathroom (245) ->Master_Bedroom (264) ->Center (279) ->Corridor (300) ->Second_Bathroom (326) 
```

A <i>fig.dot</i> file, which describes the topology of the environment should also appear in the currect folder. 
You can use the <i>dot</i> utility to create a pdf drawing of the topological map.

`dot -Tpdf fig.dot >fig.pdf`

Thus, you can verify if the topology of the environment (stored in our case in the <i>search2/etc/aruba.txt</i> file) was interpreted correctly.

### Temporal models and main benchmark

Go to the <i>fremen/src</i> folder and call <i>make</i> again. This should create a binary in the <i>fremen/bin</i> folder.
Now, to quickly evaluate the search method, call

`../bin/fremen establish ../etc/weeks_4_order_1.fre ../etc/presence.txt 2419200 3024000 60 ../../search2/etc/aruba.txt`

The arguments are following:

1.<b>establish</b> means that you are doing actual benchmarking
1.<b>../etc/weeks_4_order_1.fre</b> is the temporal model - FreMEn order 1 learnev over 4 weeks of data.
1.<b>../etc/presence.txt</b> is the person location second-by-second. 
1.<b>2419200</b> is the starting time of testing (the last second of the fourth week, i.e. 4x7x24x3600).
1.<b>3024000</b> is the end time of testing (the last second of the fifth week, i.e. (5x7x24x3600).
1.<b>60</b> is the interval of testing.

For each evaluation, you should see the expected time of search, the search path and the duration of the actual search. The benchmark also prints the mean time it took to find an object if it was in the environment (found time) and the mean time of the entire search (rescue time). 

### Evaluating results

If it works OK, then you can try to reproduce the Aruba dataset experiments by running

``./establishAruba.sh``,

which runs the search with <i>static</i>,<i>FreMEn</i> and <i>GMM</i> models and stores results in the <i>fremen-aruba</i> folder. Note, that each model evaluation takes approximatelly a minute, because it consists of 60x24x7x16 = ~160 000 searches. Since `establishAruba.sh` evaluates 7 models, the entire run can take a while.

Once the `establishAruba.sh` finishes, you can generate a latex-style table and five-point statistics by calling 

``processResults.sh``

###References

1. T.Krajnik, M.Kulich, L.Mudrova, R.Ambrus, T.Duckett: <b>[Where is waldo at time t? using spatio-temporal models for mobile robot search.](http://raw.githubusercontent.com/wiki/gestom/fremen/papers/fremen_2015_ICRA_search.pdf)</b> In proceedings of the IEEE International Conference on Robotics and Automation (ICRA), 2015. [[bibtex](http://raw.githubusercontent.com/wiki/gestom/fremen/papers/fremen_2015_ICRA_search.bib)]
