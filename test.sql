DECLARE
  vprovincia VARCHAR2(20);
  CURSOR c_sucursal IS
    SELECT codigo, poblacion
    FROM sucursal
    WHERE provincia = vprovincia;
  CURSOR c_poblacion IS
    SELECT DISTINCT poblacion
    FROM sucursal
    WHERE provincia = vprovincia;
BEGIN
    

END;
/