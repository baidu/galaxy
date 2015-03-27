
from common import http
def index(request):
    response_builder = http.ResponseBuilder()
    return response_builder.ok().build_json()


