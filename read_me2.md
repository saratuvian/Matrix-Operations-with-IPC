# Matrix Operation System

This project consists of two programs, `Ex3q2a` and `Ex3q2b`, that communicate through shared memory to perform various matrix operations such as addition, subtraction, multiplication, transpose, and logical operations (AND, OR, NOT).

## Structure

- **PLAN1**: Reads inputs from the user, stores them in shared memory, and signals `Ex3q2b` to process them.
- **PLAN2**: Reads inputs from shared memory, performs the specified matrix operations, and outputs the results.

## Matrix Input Format

Matrices are input in the following format:


- `rows`: Number of rows in the matrix.
- `cols`: Number of columns in the matrix.
- `elem`: Elements of the matrix, which can be complex numbers (e.g., `1+2i`).

## Operations

- `ADD`: Adds two matrices.
- `SUB`: Subtracts the second matrix from the first matrix.
- `MUL`: Multiplies two matrices.
- `TRANSPOSE`: Transposes the matrix.
- `AND`: Performs bitwise AND on two matrices.
- `OR`: Performs bitwise OR on two matrices.
- `NOT`: Performs bitwise NOT on a matrix.

## Termination

Both programs terminate when the `END` signal is encountered.

## Usage

1. Compile the programs:
    ```sh
    gcc -o Ex3q2a Ex3q2a.c -lpthread
    gcc -o Ex3q2b Ex3q2b.c -lpthread
    ```

2. Run the programs using the provided `run.sh` script:
    ```sh
    ./run.sh
    ```

## Example

```sh
(rows,cols:elem1,elem2,...,elemN)

### `run.sh`

```sh
#!/bin/bash

# Compile the programs
gcc -o Ex3q2a Ex3q2a.c -lpthread
gcc -o Ex3q2b Ex3q2b.c -lpthread

# Run the programs in parallel
./Ex3q2a &
./Ex3q2b &

# Wait for all background jobs to complete
wait

