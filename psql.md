# POSTGRESQL

## Меню

- [Коннект](#коннект)
- [Команды](#команды)

### Коннект
psql -U <username> -d <database_name> -h <hostname> -p <port_number>

### Команды

Команда              | Действие
-------------------- | --------------------
\l \list \l+		 | all db
\c <database_name> 	 | connect to db
\dt \dt+ 			 | all tables in db
\dv \dv+ 			 | all views in db
\dn \dn+ 			 | all schema in db
\df \df+ 			 | all functions in db
\di \di+ 			 | all indexes in db
\q 				     | quit
\! <command> 		 | shell command
\? 				     | help command
\h <command> 		 | help command
\o <filename> 		 | last response in file 
\i <filename> 		 | script from file
\echo <string> 		 | echo in console

