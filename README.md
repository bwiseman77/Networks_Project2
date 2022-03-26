# Networks_Project2

## How to use
Run `./mycalserver` to start server (use -mt for multi-threads)

Run `./mycal calendar_name action [data for action]` to run a command

Edit `.mycal` to change servername and port number

 ### examples
  
 #### Add event
`./mycal mycal add date 032722 time 1100 duration 60 name "Networks Project" description "Work on project"`
  
#### Remove event
`./mycal mycal remove 0`

#### Update event
`./mycal mycal update 0 location "cse commons"`

#### Range of days
`./mycal mycal getrange 050422 050522`

#### input file (structure shown below)
`./mycal mycal input $file_name.json`

### Structure of input file

### Structure of Response

Server will repspond with Json information about the request:
- Command : command used
- Calendar : which calendar was edited
- id : id of event
- success : if a command was successful or not (true / false)
- message : message about the request (if needed)
- data : events found with get or getrange (getrange events are numbered from 0 to n)

### Code structure

`server.c` runs the networking and then passes actions to the thread(s) and waits for more connections. The parsing of a command is in `server_func.c` which then calls the event functions in `event.c`. All source code is in `src` directory and header files in `inc`. The client is `client.py` in `src/Client`. There is also a C client, but is still a work in progress and the python client is what should be used. The python file is ran through a bash scipt `mycal`. 
