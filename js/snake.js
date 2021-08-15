const boardId = "board";
const boardBorderSize = 1;
const pointClass = "point";
const pointSize = 25;


function SnakeBody(location) {
	this.location = location;

	SnakeBody.prototype.toString = function() {
		return "SnakeBody [location=" + this.location + "]";
	}
}


function Snake() {
	this.length = 3;
	this.direction = 1;
	this.speed = 7;
	this.head = new SnakeBody(new Coordinate(7, 4));
	this.body = new Array(new SnakeBody(new Coordinate(6, 4)));
	this.tail = new SnakeBody(new Coordinate(5, 4));

	this.getLength = function() {
		return this.length;
	}

	this.move = function() {
		this.tail.location.newLocation(this.body[this.length - 3].location);

		for (let index = this.length - 3; index > 0; index--)
			this.body[index].location.newLocation(this.body[index - 1].location);

		this.body[0].location.newLocation(this.head.location);
		this.head.location.computeCoordinate(this.direction);
	}

	this.expand = function() {
		this.length++;
		this.body.push(new SnakeBody(this.tail.location.copy()));
	}

	this.hasValidLocation = function(width, height) {
		if (this.tail.location.equals(this.head.location))
			return false;

		for (let index = 0; index < this.length - 2; index++)
			if (this.body[index].location.equals(this.head.location))
				return false;

		if (!this.head.location.isOnBoard(width, height))
			return false;

		return true;
	}

	this.intersectsLocation = function(point) {
		if (this.head.location.equals(point))
			return true;

		for (let index = 0; index < this.length - 2; index++)
			if (this.body[index].location.equals(point))
				return true;

		if (this.tail.location.equals(point))
			return true;

		return false;
	}

	Snake.prototype.toString = function() {
		return "Snake [length=" + this.length + "]";
	}
}


function Point(location, element) {
	this.element = element;
	this.location = location;
	this.occupiedBy = "empty";
	this.occupiedSpcific = null;

	this.draw = function(occupiedBy, occupiedSpecific) {
		this.occupiedBy = occupiedBy;
		this.occupiedSpecific = occupiedSpecific;

		this.element.setAttribute("data-occupied-by", occupiedBy);
		this.element.setAttribute("data-occupied-specific", occupiedSpecific);
		this.element.style.backgroundColor = getColor(occupiedBy);
	}

	function getColor(occupiedBy) {
		switch (occupiedBy) {
			case "snake": return "green";
			case "food":  return "red";
			default:      return "white";
		}
	}

	Point.prototype.toString = function() {
		return "Point [location=" + this.location + "]";
	}
}


class Coordinate {
	constructor(coordX, coordY) {
		this.x = coordX;
		this.y = coordY;
	}

	equals(thisCoordinate) {
		if (this.x === thisCoordinate.x && this.y === thisCoordinate.y)
			return true;

		return false;
	}

	copy() {
		return new Coordinate(this.x, this.y);
	}

	newLocation(location) {
		this.x = location.x;
		this.y = location.y;
	}

	computeCoordinate(direction) {
		switch(direction) {
			case 0:  this.y--;
				     break;
			case 1:  this.x++;
				     break;
			case 2:  this.y++;
					 break;
			case 3:  this.x--;
				     break;
			default: break;
		}
	}

	isOnBoard(width, height) {
		if (this.x >= 0 && this.x < width && this.y >= 0 && this.y < height)
			return true;
	}

	toString() {
		return "Coordinate [x=" + this.x + ", y=" + this.y + "]";
	}
}


function Food(location) {
	this.location = location;
}


function Board(width, height) {
	this.element = document.getElementById(boardId);
	this.width = width;
	this.height = height;
	this.board = null;

	this.foodLocation = null;
	this.snakeTailLocation = null;

	this.draw = function(snake, food) {
		drawSnake(this.board, snake);
		drawFood(this.board, food);
		sendFrame(this.board, this.height, this.width);
		startStream();
	}

	this.update = function(snake, food) {
		updateSnake(this.board, snake);
		updateFood(this.board, food);
		sendFrame(this.board, this.height, this.width);
	}

	this.reset = function() {
		this.foodLocation = null;
		this.snakeTailLocation = null;

		for (let row = 0; row < this.height; row++) {
			for (let column = 0; column < this.width; column++) {
				let pointObject = this.board[row][column];
				pointObject.occupiedBy = "empty";
				pointObject.occupiedSpecific = null;
				
				let pointElement = pointObject.element;
				pointElement.setAttribute("data-occupied-by", "empty");
				pointElement.setAttribute("data-occupied-specific", null);
				pointElement.style.backgroundColor = "white";
			}
		}
	}

	function updateSnake(board, snake) {
		if (!this.snakeTailLocation.equals(snake.tail.location)) {
			findPoint(board, this.snakeTailLocation).draw("empty", null);
			findPoint(board, snake.tail.location).draw("snake", "tail");
			this.snakeTailLocation = snake.tail.location.copy();
		}

		findPoint(board, snake.head.location).draw("snake", "head");
		findPoint(board, snake.body[0].location).draw("snake", "body");
	}

	function updateFood(board, food) {
		findPoint(board, food.location).draw("food", null);
		this.foodLocation = food.location.copy();
	}

	function convertRowToNumber(row, width) {
		let power2 = 1;
		let number = 0;
		for (let col = width - 1; col >= 0; col--) {
			if (row[col].occupiedBy != "empty")
				number += power2;

			power2 *= 2;
		}

		return number;
	}

	function getFrame(board, height, width) {
		let number;
		let args = "";
		for (let row = 0; row < height; row++) {
			number = convertRowToNumber(board[row], width);
			args += "0x" + number.toString(16) + " ";
		}

		return args;
	}

    function sendFrame(board, height, width) {
        let frame = getFrame(board, height, width);

		$.ajax({
			async: true,
			url: "../php/stream.php",
			type: "post",
			data: { data: frame }
		});
    }

	function startStream() {
		$.ajax({
			async: true,
			url: "../php/output.php",
			type: "get",
			data: { mode: "stream" }
		});
	}

	this.initialiseBoard = function() {
		let boardWidth = this.width * pointSize;
		let boardHeight = this.height * pointSize;
		let minScreenWidth = boardWidth + 2 * boardBorderSize;
		if (screen.width > minScreenWidth) {
			this.element.style.width = boardWidth + "px";
			this.element.style.height = boardHeight + "px";
		}

		let pointMatrix = [];
		for (let row = 0; row < this.height; row++) {
			let pointArray = [];
			for (let column = 0; column < this.width; column++) {
				let pointElement = document.createElement("div");
				pointElement.setAttribute("class", pointClass);
				pointElement.setAttribute("data-x", column);
				pointElement.setAttribute("data-y", row);
				pointElement.setAttribute("data-occupied-by", "empty");
				pointElement.setAttribute("data-occupied-specific", "null");
				this.element.appendChild(pointElement);

				let pointLocation = new Coordinate(column, row);
				let pointObject = new Point(pointLocation, pointElement);
				pointArray.push(pointObject);
			}

			pointMatrix.push(pointArray);
		}

		this.board = pointMatrix;
	}

	function findPoint(board, location) {
		return board[location.y][location.x];
	}

	function drawSnake(board, snake) {
		let snakeHeadPoint = findPoint(board, snake.head.location);
		snakeHeadPoint.draw("snake", "head");

		for (let index = 0; index < snake.length - 2; index++) {
			let snakeBodyPoint = findPoint(board, snake.body[index].location);
			snakeBodyPoint.draw("snake", "body");
		}

		let snakeTailPoint = findPoint(board, snake.tail.location);
		snakeTailPoint.draw("snake", "tail");
		this.snakeTailLocation = snake.tail.location.copy();
	}

	function drawFood(board, food) {
		let foodPoint = findPoint(board, food.location);
		foodPoint.draw("food", "null");
		this.foodLocation = food.location.copy();
	}

	Board.prototype.toString = function() {
		return "Board [width=" + this.length + ", height=" + this.height + "]";
	}
}


class Game {
	constructor() {
		this.width = 24;
		this.height = 8;
		this.running = false;
		this.pause = false;
		this.lastRenderTime = 0;
		this.inputQueue = [];
		this.score = 0;
		this.scoreElement = document.getElementById("score");

		this.snake = new Snake();
		this.food = new Food(this.getFoodNewLocation(this.snake, this.width, this.height));
		this.board = new Board(this.width, this.height);
	}

	initialiseGame() {
		this.scoreElement.innerHTML = this.score;
		this.board.initialiseBoard();
		this.board.draw(this.snake, this.food);

		window.addEventListener("keydown", (e) => {
			let thisSnakeDirection = this.snake.direction;
			let inputQueueLength = this.inputQueue.length;
			switch (e.key) {
				case 'ArrowUp':
					if (inputQueueLength > 0 && this.inputQueue[inputQueueLength - 1] != 2 || thisSnakeDirection != 2)
						this.inputQueue.push(0);
					break;
				case 'ArrowRight':
					if (inputQueueLength > 0 && this.inputQueue[inputQueueLength - 1] != 3 || thisSnakeDirection != 3)
						this.inputQueue.push(1);
					break;
				case 'ArrowDown':
					if (inputQueueLength > 0 && this.inputQueue[inputQueueLength - 1] != 0 || thisSnakeDirection != 0)
						this.inputQueue.push(2);
					break;
				case 'ArrowLeft':
					if (inputQueueLength > 0 && this.inputQueue[inputQueueLength - 1] != 1 || thisSnakeDirection != 1)
						this.inputQueue.push(3);
					break;
				case 'p':
					if (this.pause)
				    	this.pause = false;
				    else
				    	this.pause = true;
				    break;
				default:
					break;
			}
		});
	}

	run(currentTime) {
		if (!this.running) {
			alert("Game Over");
			this.board.reset();
			this.lastRenderTime = 0;
			this.score = 0;
			this.scoreElement.innerHTML = this.score;
			this.snake = new Snake();
			this.food.location = this.getFoodNewLocation(this.snake, this.width, this.height);
			this.board.draw(this.snake, this.food);
			this.running = true;
		}

		window.requestAnimationFrame((e) => { this.run(e); });
		const secondsSinceLastRender = (currentTime - this.lastRenderTime) / 1000;
		if (secondsSinceLastRender < 1 / this.snake.speed)
			return;

		if (!this.pause) {
			if (this.inputQueue.length > 0) {
				this.snake.direction = this.inputQueue[0];
				this.inputQueue.shift();
			}
			this.snake.move();
			if (this.snake.hasValidLocation(this.width, this.height)) {
				if (this.snake.intersectsLocation(this.food.location)) {
					this.food.location = this.getFoodNewLocation(this.snake, this.width, this.height);
					this.snake.expand();

					this.increaseScore();

				}
				this.board.update(this.snake, this.food);
			}
			else
				this.running = false;
		}

		this.lastRenderTime = currentTime;
	}

	increaseScore() {
		this.score++;
		this.scoreElement.innerHTML = this.score;
	}

	getFoodNewLocation(snake, width, height) {
		let newLocation = this.getRandomCoordinate(width, height);
		while (snake.intersectsLocation(newLocation))
			newLocation = this.getRandomCoordinate(width, height);

		return newLocation;
	}

	getRandomCoordinate(largestX, largestY) {
		return new Coordinate(this.getRandomInt(largestX), this.getRandomInt(largestY));
	}

	getRandomInt(max) {
  		return Math.floor(Math.random() * Math.floor(max));
	}

	sleep(ms) {
  		return new Promise(resolve => setTimeout(resolve, ms));
	}
}


function main() {
	let game = new Game();
	game.initialiseGame();
	game.running = true;
	window.requestAnimationFrame((e) => { game.run(e); });
}

main();