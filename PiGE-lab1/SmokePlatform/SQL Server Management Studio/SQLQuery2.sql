--QUERY 1
select * from Employees e
join Orders o on o.EmployeeID = e.EmployeeID
join [Order Details] od on od.OrderID = o.OrderID
WHERE o.ShipCountry LIKE 'S%' OR o.ShipCountry LIKE 'F%'
GROUP BY e.EmployeeID, o.OrderID, e.FirstName
HAVING COUNT(DISTINCT o.OrderID) >= 2 AND MAX(od.Quantity) <=3
ORDER BY e.FirstName ASC


--QUERY 2
SELECT  e.FirstName, p.ProductName  
FROM Employees e
JOIN Orders o ON e.EmployeeID = o.EmployeeID 
JOIN [Order Details] od ON od.OrderID = o.OrderID 
JOIN Products p ON p.ProductID = od.ProductID
WHERE  od.Quantity > 0
GROUP BY e.FirstName, e.LastName, p.ProductName
HAVING SUM(od.Quantity) > (SELECT AVG(SubQuery.AvgQuantity) * 0.6 
FROM (SELECT p.ProductName, AVG(od.Quantity) AS AvgQuantity 
FROM [Order Details] od
JOIN Products ON od.ProductID = p.ProductID 
GROUP BY Products.ProductName)
AS SubQuery 
WHERE 
SubQuery.ProductName = p.ProductName)
ORDER BY e.LastName, e.FirstName


--QUERY 3
--SELECT o.*
--FROM Orders o
--JOIN Employees e ON e.EmployeeID = o.EmployeeID
--WHERE e.EmployeeID IN (SELECT TOP 2 e2.EmployeeID FROM Employees e2 ORDER BY e2.HireDate ASC)
--AND o.OrderDate = ( SELECT MAX(o2.OrderDate) FROM Orders o2 WHERE o2.EmployeeID = e.EmployeeID
--);

--QUERY 4

--SELECT c.CategoryName from Categories c
--JOIN Products p ON c.CategoryID = p.CategoryID
--JOIN [Order Details] od ON p.ProductID = od.ProductID
--JOIN Orders o ON od.OrderID = o.OrderID
--GROUP BY p.ProductName, c.CategoryName
--HAVING Sum(CASE WHEN month(o.OrderDate)=3 OR month(o.OrderDate)=4 AND year(o.OrderDate)=1996 THEN 1 ELSE 0 END)>=Sum(CASE WHEN month(o.OrderDate)=3 OR month(o.OrderDate)=4 AND year(o.OrderDate)=1997 THEN 1 ELSE 0 END)
--ORDER BY Count(DISTINCT o.OrderID)

