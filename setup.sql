-- Ensures the existence of the table
CREATE TABLE IF NOT EXISTS movies (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    release_year INTEGER,
    genres TEXT,
    director TEXT
);

DELETE FROM movies;

-- First movies
INSERT INTO movies (title, release_year, genres, director)
VALUES 
('Palhaços Assassinos do Espaço Sideral', 1988, 'SciFi,Comédia,Paródia,Horror', 'Stephen Chiodo'),
('O VelociPastor', 2018, 'Filme-B,Ação,Comédia,Horror', 'Brendan Steere'),
('Deus e o Diabo na Terra do Sol', 1964, 'Crime,Drama', 'Glauber Rocha'),
('Monty Python em Busca do Cálice Sagrado', 1975, 'Comédia,Paródia,Sátira,Aventura,Fantasia', 'Terry Gilliam'),
('A Viagem de Chihiro', 2001, 'Animação,Fantasia,Aventura', 'Hayao Miyazaki'),
('Ainda Estou Aqui', 2024, 'Drama,História,Biografia,Político', 'Walter Salles'),
('Poltergeist: O Fenômeno', 1982, 'Horror,Sobrenatural,Suspense', 'Tobe Hooper');