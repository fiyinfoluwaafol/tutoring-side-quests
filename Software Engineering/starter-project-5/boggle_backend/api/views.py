from rest_framework.decorators import api_view
from rest_framework.response import Response
from rest_framework import status
from .models import Games
from .serializer import GamesSerializer
from random import randrange
from .randomGen import *
from .readJSONFile import *
from .boggle_solver import *
from django.contrib.staticfiles import finders
from datetime import datetime

# define the endpoints

@api_view(['GET', 'DELETE']) # define a GET Object with pk
def get_game(request, pk):
    try:
        game = Games.objects.get(pk=pk)
    except Games.DoesNotExist:
        return Response(status=status.HTTP_404_NOT_FOUND)
    
    if request.method == 'GET':
        serializer = GamesSerializer(game)
        return Response(serializer.data)
    elif request.method == 'DELETE':
        game.delete()
        return Response(status=status.HTTP_204_NO_CONTENT)
 
@api_view(['GET']) # define a GET REQUEST to get ALL Games
def get_games(request):
    games = Games.objects.all()
    serializer = GamesSerializer(games, many=True)
    return Response(serializer.data)

@api_view(['GET']) # define a PUT REQUEST TO ADD A SPECIFIC GAME OF SIZE size
def create_game(request, size):
    if((size <= 10) and (size >= 3)):
        g = random_grid(size)
        now = datetime.now()
        name = f'Rand{size}Grid:{now.strftime("%Y-%m-%d %H:%M:%S")}'

        # Find the absolute path of the static JSON file
        file_path = finders.find("data/full-wordlist.json")

        dictionary = read_json_to_list(file_path)    
        mygame = Boggle(g, dictionary)
        fwords = mygame.getSolution()

        serializer = GamesSerializer(data={"name": name,"size": size, "grid": str(g), "foundwords": str(fwords)})
        print("Done serializing")
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data, status=status.HTTP_201_CREATED)
    print("Invalid")
    return Response(serializer.data, status=status.HTTP_400_BAD_REQUEST)