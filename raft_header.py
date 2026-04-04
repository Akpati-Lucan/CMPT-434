
from enum import IntEnum
####################################################################################
#Label parameters - The label instructs which ever process receives about what the message is for
####################################################################################

class Message:
    def __init__(self, label, seq_number, source_name, source_port,
                 dest_name, dest_port, msg="", vote_for=0):

        self.label = label
        self.seq_number = seq_number
        self.source_name = source_name
        self.source_port = source_port
        self.dest_name = dest_name
        self.dest_port = dest_port
        self.msg = msg
        self.vote_for = vote_for


class Server:
    def __init__(self, name, port, is_leader=False):
        self.name = name
        self.port = port
        self.is_leader = is_leader


####################################################################################

class Label(IntEnum):
    NEW_LOG_VALUE = 0
    APPEND = 1
    APPEND_ACK = 2
    REJECT = 3
    COMMIT = 4
    HEARTBEAT = 5
    HEARTBEAT_ACK = 6
    VOTE = 7
    VOTE_FOR = 8
    NEW_LEADER = 9
    UPDATE_CHECK = 10
    UPDATE_SIGNAL = 11
    UPDATE = 12
    UPDATE_ACK = 13
    ADD_SERVER = 14
    NEW_SERVER = 15



