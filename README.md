# Key Value Store
a simple key value store with CRUD operations

## Architecture
The server uses two different message queues, one for reading operations and the other for response operations. During initialization, the server sets up a pool of worker threads to handle incoming connections. All request processing is **ATOMIC**, since the server can handle requests from multiple clients or multi-threaded client (as the example client in the repository works). 

## Usage
### Command line args
```
  -d [dcount]: number of dataset files (more files means lesser locks on transactions, one transaction locks one dataset file)
  if (args->fname)
  -f [fname]: name/path prefix of the dataset files
  -t [tcount]: maximum number of worker threads
  -s [vsize]: fixed size of the values to store (padded with null characters or truncates to length)
  -m [mqname] : request message queue, response message queue is suffixed with '-resp'
```
### Running the example
```
$ make
$ make server-run
$ make client-run-t (for interactive mode)
$ make client-run (for batch processing of spec files)
```

## Supported operations
1. Read Value of Key
```
GET 100
[response] success : 1, value : THIS
GET 101
[response] success : 0, value : (nil)
```
2. Add new Key Value pair
```
PUT 101 THAT
[response] success : 1, value : (nil)
```
3. Update Value of Key in place
```
/* if the key exists, it's value is updated */
PUT 101 THOSE
[response] success : 1, value : (nil)
```
4. Delete Key Value pair
```
DEL 100
[response] success : 1, value : (nil)
DEL 102
[response] success : 0, value : (nil)
```