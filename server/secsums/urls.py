from django.urls import path
from . import views


urlpatterns = [
    path("<str:name>/ready", views.ready, name="ready"),
    path("<str:name>/<int:step>", views.secsum, name="secsum"),
    path("<str:name>/stats", views.statistic_website, name="stats"),
    path("<str:name>/stats_refresh", views.statistic_json, name="stats_refresh"),
    path("<str:name>/result", views.result, name="result"),
]
