from enum import Enum


class ConnectionTypes(int, Enum):
    CLIENT_TO_SERVER = 100
    PRIMARY_SERVER_TO_CLIENT = 101
    SECONDARY_SERVER_TO_CLIENT = 102


ConnectionTypeStrings = {
    ConnectionTypes.CLIENT_TO_SERVER: "Client to server",
    ConnectionTypes.PRIMARY_SERVER_TO_CLIENT: "Primary server to client",
    ConnectionTypes.SECONDARY_SERVER_TO_CLIENT: "Secondary server to client",
}
