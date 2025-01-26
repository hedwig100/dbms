# Data on files

This documents describes how data is arranged on files.

## Table

A file holds all data on a table.

## Layout on blocks
Each table is stored in a file.
The file is divided into blocks by block size. Each record has the same length and is arranged so that this does not span blocks.

In each block, pairs of a empty flag and record are arranged, and space left in the block is not used.

```
| empty flag (1byte) | record | empty flag (1byte) | record | ... | not used |
```

When a empty flag is 0x01, the record is not empty and when that is 0x00, the record is empty.

## Record Layout

The record layout is decided so that the size is the smallest.