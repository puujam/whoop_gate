import json
from pymongo import MongoClient

config = json.loads( "../../config.json" )
client = MongoClient( config[ "mongo_connection_string" ] )

class WhoopMongoConfig():
    def __init__( self ):
        self.node = client.db.config

    @property
    def gate_order( self ):
        return self.node[ "gate_order" ]
