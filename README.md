# Satisfactory Factory

The project is an optimization tool for the video game **Satisfactory**. And it aims to ultra optimize factories based on resource nodes on the map and factory nodes on the map.

The end goal of this project is to decide optimal placement of new factory nodes in your production chain, and to decide the best recipes and machine ratios to use, given available item streams and desired output streams.

With this tool, you can cover the entire map in factories that are **gloriously optimal!**

But, as it is unfinished; we highly recommend you check out https://www.satisfactorytools.com/ and https://satisfactory-calculator.com/ for your day to day factory needs.





#### Current State of the project

The project has just hit its first big milestone! And it is now public.

Given the *en-US.json* file from the **Satisfactory** game folder, the program can filter and examine combinations of alternate recipes in the recipe chains for items.

This means, that the next milestone will be the construction of factory nodes from input output specifications!





**Past Milestones**

Recorded all resource nodes on the Satisfactory Map

Constructed an node graph of recipes (see Obsidian)





#### About the Project

Resource node data was recorded from an online map viewer: https://satisfactory-calculator.com/en/interactive-map.

This project reformats the internal recipe list of **Satisfactory** then constructs a node graph.

This graph is then prepared with topological sort, so that less complex items can be processed first.

The node graph is then traversed with a BFT algorithm that examines all combinations of alternate recipes for each part of the recipe chain. A filter is applied dynamically to prevent "divergence."

The result of this algorithm is a list of condensed recipes for each item that include the item input and output streams needed to operate one factory, which uses whole number machine ratios, producing that item from the ground up.





#### Contributors

Vrenteeror-Karnvak - Project Lead and Head Programmer

Bartimaues812 - Support Staff
