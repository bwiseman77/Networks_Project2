import sys, socket, json

BUFSIZ = 4096
Commands = {"add", "get", "remove", "update", "getrange"}
Fields = {"date", "time", "duration", "name", "location", "description"}


def main():
    if len(sys.argv) == 1:
        print("No calendar specified")
        help()
        exit(-1)
    elif len(sys.argv) == 2:
        print("No command specified")
        help()
        exit(-1)

    event = None
    error = None
    calendar = sys.argv[1]
    cmd = sys.argv[2]
    if cmd == "add":
        calendarEntry = {"date": None, "time": None, "duration": None, "name": None}
        for i in range(3, len(sys.argv), 2):
            if sys.argv[i] not in Fields:
                error = "Client Error: Invalid field name"
            elif len(sys.argv) == i + 1:
                error = "Client Error: No value entered for this field"
            else:
                calendarEntry[sys.argv[i]] = sys.argv[i+1]
        for field in calendarEntry:
            if calendarEntry[field] == None:
                error = f"Required field unspecified: {field}"

        if not isValidDate(calendarEntry["date"]):
            error = "Client Error: Invalid date"
        elif not isValidTime(calendarEntry["time"]):
            error = "Client Error: Invalid time"
        elif not isValidDuration(calendarEntry["duration"]):
            error = "Client Error: Invalid dutration"
        else:
            event = calendarEntry
    elif cmd == "get":
        if len(sys.argv) != 4:
            error = "Client Error: Incorrect number of arguments"
        else:
            event = {"date": sys.argv[3]}
    elif cmd == "remove":
        if len(sys.argv) != 4:
            error = "Client Error: Incorrect number of arguments"
        else:
            event = {"id": sys.argv[3]}
    elif cmd == "update":
        if len(sys.argv) != 6:
            error = "Client Error: Incorrect number of arguments"
        else:
            event = {"id": sys.argv[3], sys.argv[4]:sys.argv[5]}
    elif cmd == "getrange":
        if len(sys.argv) != 5:
            error = "Client Error: Incorrect number of arguments"
        else:
            event = {"startDate": sys.argv[3], "stopDate":sys.argv[4]}
    else:
        error = "Client Error: Invalid command"

    # print(response)
    host = "localhost"
    port = 41256

    if error == None:
        client_socket = connect(host, port)
        print(f"connected to {host}")

        r = send_message(client_socket, cmd, event)
        response = json.loads(r[:len(r)-1]) # trim null charachter
        print(response)

        close(client_socket)
    else:
        print(error)

def isValidDate(date):
    if len(date) == 6 and date.isnumeric():
        if int(date[:2]) <= 12 and int(date[:2]) >= 1:
            if int(date[2:4]) <= 31 and int(date[2:4]) >= 1:
                return True
    return False

def isValidTime(time):
    if len(time) == 4 and time.isnumeric():
        if int(time[:2]) < 24:
            if int(time[2:]) < 60:
                return True
    return False

def isValidDuration(duration):
    return duration.isnumeric()

def connect(host, port):
    client_socket = socket.socket()
    client_socket.connect((host, port))
    return client_socket


def close(client_socket):
    client_socket.close()

def send_message(client_socket, cmd, event):
    message = bytes(f"{cmd} {json.dumps(event)}", "utf-8")
    print(f'sending: {cmd} {json.dumps(event)}')
    client_socket.send(message)
    data = client_socket.recv(BUFSIZ)
    return data

def help():
    print("Usage:    ./mycal CalendarName action -> data for action <-")


if __name__ == "__main__":
    main()
