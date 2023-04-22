--select * from Orders where EmployeeID in (1,4)
--update orders
--set EmployeeID=4
--where EmployeeID=1

--can also be done with begin transaction and --rollback transaction and the start and end

update [Order Details]
set Quantity= round(0.8*Quantity, 0) 
where (select OrderDate from Orders o where [Order Details].OrderID = o.OrderID) > '1997-05-15' and
(select ProductID from Products p where ProductName = 'Ikura') = ProductID
--update [Order Details] set Quantity= 0.80*Quantity 


select od.* from  Orders o
join [Order Details] od on o.OrderID = od.OrderID
join Products p on od.ProductID = p.ProductID
where o.CustomerID='ALFKI' and p.ProductName= 'Chocolade'
order by o.OrderDate DESC

begin transaction
insert into [Order Details] (OrderID, ProductID,Quantity)
select top(1) o.OrderID,
(select p.ProductId