from django.urls import path
from . import views


urlpatterns = [
    path("start_secsums/<int:num_clients>", views.start_secsums, name="start_secsums"),
    path("start_dummysums/<int:num_clients>", views.start_dummysums, name="start_dummysums"),
]
