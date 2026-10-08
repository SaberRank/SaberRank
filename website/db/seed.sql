-- Optional development seed for SnoreSaber 3.0.
-- Run after schema.sql if you want a populated local/production test database.
INSERT INTO players (id, steam_id, name, country, avatar, pp, rank, country_rank)
VALUES
('76561198000000001','76561198000000001','YawningSylveon','CA','/assets/snoresaber-icon.png',16284.32,1,1),
('76561198000000002','76561198000000002','Lunaa','US','/assets/snoresaber-icon.png',16112.07,2,1),
('76561198000000003','76561198000000003','Kyouki','JP','/assets/snoresaber-icon.png',15998.44,3,1),
('76561198000000004','76561198000000004','Rho','US','/assets/snoresaber-icon.png',15781.20,4,2),
('76561198000000005','76561198000000005','Astra','GB','/assets/snoresaber-icon.png',15662.11,5,1)
ON CONFLICT (id) DO NOTHING;

INSERT INTO maps (id, hash, song_name, song_author_name, level_author_name, bpm, cover_url, verified)
VALUES
(1,'A1B2C3D4E5F6','Imprinting','CreepyBlock','Sotarks',174,'/assets/snoresaber-icon.png',true),
(2,'B2C3D4E5F6A1','B.B.K.K.B.K.K.','nora2r','nora2r',170,'/assets/snoresaber-icon.png',true),
(3,'C3D4E5F6A1B2','Kimi no Bouken','Sotarks','Sotarks',168,'/assets/snoresaber-icon.png',true),
(4,'D4E5F6A1B2C3','Ghost','Camellia','Rustic',150,'/assets/snoresaber-icon.png',true),
(5,'E5F6A1B2C3D4','Machine Gun','Kobaryo','Sotarks',200,'/assets/snoresaber-icon.png',true)
ON CONFLICT (id) DO NOTHING;

SELECT setval(pg_get_serial_sequence('maps','id'), GREATEST((SELECT COALESCE(MAX(id),1) FROM maps),1));

INSERT INTO leaderboards (id,map_id,difficulty,game_mode,raw_difficulty,max_score,stars,status,ranked_at)
VALUES
(10,1,9,'Standard','ExpertPlus',1000000,9.42,'RANKED',now()),
(20,2,9,'Standard','ExpertPlus',1000000,9.18,'RANKED',now()),
(30,3,9,'Standard','ExpertPlus',1000000,8.91,'RANKED',now()),
(40,4,9,'Standard','ExpertPlus',1000000,8.74,'RANKED',now()),
(50,5,9,'Standard','ExpertPlus',1000000,8.62,'RANKED',now())
ON CONFLICT (id) DO NOTHING;

SELECT setval(pg_get_serial_sequence('leaderboards','id'), GREATEST((SELECT COALESCE(MAX(id),1) FROM leaderboards),1));
