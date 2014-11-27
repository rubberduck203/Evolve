**Evolve** is an artificial life simulator that I found a few years ago. 
Since I originally found it, the website has gone offline. 
I am posting the source and installer here in order to preserve the project for posterity.

Below is the old project homepage, slightly modified to deal with broken links.
The website, in it's entirety is available in the `evolve4www` directory.
It includes a quickstart and full manual.

[Installer available here](https://github.com/ckuhn203/Evolve/releases/tag/v4.0)

---

<!--Begin main text-->
<div id="maintext" height="600px">

<H1>EVOLVE</H1>

<h3>About this Software</h3>
<p>
<B>Evolve 4.0</B> is a simulator of evolution using a
simplified 2-dimensional universe. This software lets you create
new simulations, run them, and visualize the behavior of the evolving creatures.

<P>
The first version of this software was created in 1996. Fortunately, it has
evolved (just like the creatures that live in the simulator) into an awsome
piece of software. The most recent version  is a complete windows application with a well designed
GUI and lots of features for examining the evolved creatures.
<P>
<HR>
<P>

<h3>Installation</h3>
<OL>
<LI> Download the application by right clicking on <A HREF="evolve4exe.zip">this link</A> and saving
the ZIP file to your hard disk.
<P>

<LI> Double click on the ZIP file and extract the file 'setup.exe' to your hard drive.
<P>

<LI> Run 'setup.exe'.
<P>

<LI> Follow the instructions from the setup program.
<P>

<LI> That's it! Now you can run the "Evolve" application from your Start Menu.
<P>

</OL>

<HR>

<h3>Requirements</h3>
<P>
The GUI software runs on Windows XP. The
command line utility (no graphics) runs on both Windows and Linux.
This application is written in C/C++ using Visual Studio .NET. Source code is available, as
well as executable.
<P>
Oh yeah, one more requirement: You will need a complete Bio Hazard Level 4 facility to run this
software.
<P>

<HR>

<h3>Genre</h3>
This program is a simulator of artificial life. A virtual universe is
created that supports the basics of evolution: Replicators and cummulative selection.
On the Internet, the ALIFE community includes a lot of software all trying to do related things.
There are two main categories of ALIFE software:

<OL>
<LI><B>Primordial Soup simulations:</B> These simulators begin with simple rules and
attempt to cause the emergence of replicators. (I.e, Conways Life)
<P>

<LI><B>Artifical life:</B> The machinery for replication is the starting point. And the
goal is to evolve better and better replicators.
</OL>

<P>
<B>Evolve 4.0</B> is most definitely an example of (2). Replication and mutations are built
into the system. One of its design goals was to partially simulate earth-based life.
Evolve tries to emulate the same structure of cells and organisms as one finds in nature.

<P>
This simulator is a union of <I>Conway's Game of Life</I> and <I>Core Wars</I>.
The game of life presents a very simple two dimenional universe, with some basic rules that
cells in the universe follow (Evolve has a slightly more complex set of rules).
Core wars is a virtual computer running many programs (written in REDCODE)
in a shared memory space (called core). Each program tries to copy itself and
crowd out other programs.

<P>
Evolve uses a sophisticated forth-like language (called KFORTH) for the control of organisms and
cells. This is the analog of our DNA. The size of these programs is unbounded and
therefore very complex behaviors can evolve. KFORTH programs do not run in a shared
memory space rather, each cell has its own virtual computing machine that includes a
data and call stack plus some working registers. An organism can have many cells all
executing different parts of the same genetic program. It is therefore possible for cells to specialize
their function, and thus achieve even more complex behaviors.
<P>

<H4>Differences from other ALife software:</H4>
Most artifical life simulators use a very restricted genetic layout. One program for
example, called "Darwin Pond" has about 8 or 9 genes which are simple floating
point numbers. All the logic for reproduction and consumption and movement are
hard coded in the simulation. There is very little genetic variation that can occur.
The genetic programming language for Evolve is unlimited in size and therefore extremely
advanced behavior for organisms can emerge.
<P>
Biomorphs also have a limited set of genes, and the selection process is through user
interaction. In the Evolve program, the selection process is part of the simulation.
<P>
Other ALife simulators run short scenarios with a small handful of organisms, and then after
a fixed period of simulation time, the best organisms are retained and the rest are
eliminated. Then a new scenario is started. The Evolve simulator is more realistic.
In Evolve there is a virtual universe in which thousands of organisms live and reproduce. The
simulations you will run are intended to run for billions of simulations
steps, with billions of births and deaths.

<P>
Evolve has been in development since 1996. Read about the history of Evolve 4.0
<A HREF="history.html">here</A>.

<P>
<HR>

<h3>License</h3>
<P>
The Evolve application is completely free. The <A HREF="evolve4src.zip">source code</A> can be
used according to this license:

<PRE>
Evolve, Version 4.0
URL: http://www.stauffercom.com/evolve4
E-mail: ken@stauffercom.com

Copyright (C) 1996-2006 by Ken Stauffer.
All rights reserved.
 Eiffel Forum License, version 2

   1. Permission is hereby granted to use, copy, modify and/or
      distribute this package, provided that:
          * copyright notices are retained unchanged,
          * any distribution of this package, whether modified or not,
      includes this license text.
   2. Permission is hereby also granted to distribute binary programs
      which depend on this package. If the binary program depends on a
      modified version of this package, you are encouraged to publicly
      release the modified version of this package.

***********************

THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT WARRANTY. ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHORS BE LIABLE TO ANY PARTY FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES ARISING IN ANY WAY OUT OF THE USE OF THIS PACKAGE.

***********************
</PRE>

</div>

<div id="footer">
<a href="mailto:ken@stauffercom.com">E-mail</a> | <a href="http://www.stauffercom.com">Home</a>
<br />
&copy; 2006 Ken Stauffer. All rights reserved.
</div>


