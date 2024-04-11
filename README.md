[![INFORMS Journal on Computing Logo](https://INFORMSJoC.github.io/logos/INFORMS_Journal_on_Computing_Header.jpg)](https://pubsonline.informs.org/journal/ijoc)

# Solving the Minimum Sum Coloring Problem: Alternative Models, Exact Solvers, and Metaheuristics

This archive is distributed in association with the [INFORMS Journal on
Computing](https://pubsonline.informs.org/journal/ijoc) under the [MIT License](LICENSE).

The software and data in this repository are a snapshot of the software and data that were used in the research reported on in the paper Solving the Minimum Sum Coloring Problem: Alternative Models, Exact Solvers, and Metaheuristics by Y. Du, F. Glover, G. Kochenberger, R. Hennig, H. Wang and A. Hulandageri.


## Cite

To cite the contents of this repository, please cite both the paper and this repo, using their respective DOIs.

https://doi.org/10.1287/ijoc.2022.0334

https://doi.org/10.1287/ijoc.2022.0334.cd

Below is the BibTex for citing this snapshot of the respoitory.

```
@article{A Fusion Pre-Trained Approach for Identifying the Cause of Sarcasm Remarks,
  author =        { Y. Du, F. Glover, G. Kochenberger, R. Hennig, H. Wang and A. Hulandageri },
  publisher =     {INFORMS Journal on Computing},
  title =         { Solving the Minimum Sum Coloring Problem: Alternative Models, Exact Solvers, and Metaheuristics },
  year =          {2024},
  doi =           {10.1287/ijoc.2022.0334.cd},
  url =           {https://github.com/INFORMSJoC/2022.0334},
}
```

## Description

This repository provides data for the problem and scripts for the models.

The main folders are 'data', 'scripts', and 'results'.
- '[data](data)': This folder includes minimum sum graph coloring data sets.
- '[scripts](scripts)': This folder provides a running script.
- '[results](results)': This folder provides the results.

## Building

This directory includes the [scripts](scripts) for generating and solving four models using Gurobi.

The following packages should be installed before you run our model.

```
python >= 3.8.13
gurobi >=9.5
```
## Results

The [results](results) folder show the tables reported in the paper.

[Table 1](results/Table%201.pdf) shows characteristic of tested data.

[Table 2](results/Table%202.pdf) presents the computational results of four models using exact solver Gurobi.

[Table 3](results/Table%203.pdf) shows the comparison results of four models using Exact and Metaheuristic Solvers.
