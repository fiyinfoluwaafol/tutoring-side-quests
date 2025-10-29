"""
Name: Fiyinfoluwa Afolayan
SID: @03054145
"""

class Boggle:
    def __init__(self, grid, dictionary):
        """Initialize the Boggle game with a grid and dictionary."""
        self.setGrid(grid)
        self.setDictionary(dictionary)
        self.solution_set = set()
        self.buildTrie()
        self.rows = len(self.grid)
        self.cols = len(self.grid[0]) if self.rows > 0 else 0

    def setGrid(self, grid):
        """Set the grid and convert all tiles to uppercase."""
        self.grid = [[tile.upper() for tile in row] for row in grid]

    def setDictionary(self, dictionary):
        """Set the dictionary and convert all words to uppercase."""
        self.dictionary = [word.upper() for word in dictionary]

    def getSolution(self):
        """Get the list of found words."""
        self.findWords()
        return list(self.solution_set)

    def buildTrie(self):
        """Build a Trie from the dictionary."""
        self.trie = {}
        for word in self.dictionary:
            node = self.trie
            for char in word:
                node = node.setdefault(char, {})
            node['#'] = True  # '#' marks the end of a word

    def findWords(self):
        """Find all words in the grid that are in the dictionary."""
        self.solution_set = set()
        for i in range(self.rows):
            for j in range(self.cols):
                self.dfs(i, j, self.trie, "", set())

    def get_next_node(self, node, tile_string):
        """Get the next node in the trie after consuming the tile_string."""
        current_node = node
        for char in tile_string:
            if char in current_node:
                current_node = current_node[char]
            else:
                return None
        return current_node

    def dfs(self, i, j, node, path, visited):
        """Perform DFS to find words starting from cell (i, j)."""
        if (i < 0 or i >= self.rows or j < 0 or j >= self.cols or (i, j) in visited):
            return
        tile_string = self.grid[i][j]
        next_node = self.get_next_node(node, tile_string)
        if not next_node:
            return
        visited.add((i, j))
        path += tile_string
        if '#' in next_node and len(path) >= 3:
            self.solution_set.add(path)
        # Explore all adjacent cells
        for x in range(i - 1, i + 2):
            for y in range(j - 1, j + 2):
                if (x, y) != (i, j):
                    self.dfs(x, y, next_node, path, visited)
        visited.remove((i, j))


def main():
    grid = [['A', 'B', 'C', 'D'],
            ['E', 'F', 'G', 'H'], 
            ['I', 'J', 'K', 'L'], 
            ['A', 'B', 'C', 'D']]

    dictionary = ['ABEF', 'AFJIEB', 'DGKD', 'DGKA']

    mygame = Boggle(grid, dictionary)
    print(mygame.getSolution())

if __name__ == "__main__":
  main()
