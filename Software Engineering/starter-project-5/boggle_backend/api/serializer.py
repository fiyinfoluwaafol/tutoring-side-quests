from rest_framework import serializers
from .models import Games

# creating a model class below
class GamesSerializer(serializers.ModelSerializer):
    class Meta:
            model = Games
            fields = '__all__'
    
    def validate_size(self, value):
        if value < 3 or value > 10:
            raise serializers.ValidationError("Size must be between 3 and 10.")
        return value

    def validate(self, data):
        if not data.get('grid'):
            raise serializers.ValidationError("Grid cannot be empty.")
        if not data.get('foundwords'):
            raise serializers.ValidationError("Foundwords cannot be empty.")
        return data