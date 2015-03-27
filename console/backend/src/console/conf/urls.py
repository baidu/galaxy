# -*- coding:utf-8 -*-
from django.conf import urls

#views
urlpatterns = urls.patterns("console.conf.views",
        (r'^get','get_conf'),
)

