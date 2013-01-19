# ctst

`ctst` is an implementation of a [Trie](http://en.wikipedia.org/wiki/Trie), a
data structure mapping string-like key to any kind of value. More specifically,
it implements a close variant of a [radix tree](http://en.wikipedia.org/wiki/Radix_tree)
with [B-Tree](http://en.wikipedia.org/wiki/B-tree)-like storage of node pointers.

When storing this list of words :

```
colored
colours
comitee
community
commuter
commuters
compacity
compact
compacted
compute
continuity
```

`ctst` builds this tree :

![Sample graph](https://raw.github.com/nlehuen/ctst/master/doc/graph.png)

The differences between `ctst` and another Trie project, [pytst](https://github.com/nlehuen/pytst)
are discussed in [this article](http://nicolas.lehuen.com/2009/04/19/the-difference-between-pytst-and-ctst-4/).
