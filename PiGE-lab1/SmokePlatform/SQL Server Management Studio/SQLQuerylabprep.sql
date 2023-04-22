--select ProductName AS Product_Name from Products
--where ProductName LIKE 'S%'
--order by ProductName DESC


--select top 5 country from Customers c 
--join Orders o on c.CustomerID = o.CustomerID
--WHERE YEAR(OrderDate)=1998 group by c.Country order by Count(OrderID) desc

select c.CustomerID from Customers c
join orders o on o.CustomerID = c.CustomerID
join [Order Details] od on od.OrderID = o.OrderID
join Products p on p.ProductID = od.ProductID
where p.ProductName = 'Ipoh Coffee'
group by c.CustomerID having SUM(quantity) > AVG(quantity)


