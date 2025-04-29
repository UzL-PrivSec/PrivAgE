#!/bin/bash

. server_env/bin/activate

python3 manage.py makemigrations
python3 manage.py migrate

deactivate
