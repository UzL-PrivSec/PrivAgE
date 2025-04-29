from django.urls import path
from . import views


urlpatterns = [
    path("<str:name>/ready", views.ready, name="ready"),
    path("<str:name>/<int:step>", views.dummysum, name="dummysum"),
    path("<str:name>/result", views.result, name="result"),
]
