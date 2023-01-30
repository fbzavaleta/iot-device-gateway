from flask import jsonify
from schematics import Model as SchematicModel
from schematics.types import StringType, DictType


class   ApiType(SchematicModel):
    device  =   StringType(required=True)
    date_time   =   StringType(required=True)

    def to_response(self):
        return jsonify(self.to_primitive()) 


class   ChannelFeed(ApiType):
    data    =   DictType(StringType(), default=dict)
    status  =   StringType()
    

class   ChannelStatus(ApiType):
    number_request  =   StringType(required=True)

class ProcessingType():
    pass
