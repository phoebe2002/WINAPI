select ProductName from Products
join [Order Details] on Products.ProductID = Orders.ProductID
join Orders on [Order Details].OrderID = Orders.OrderID
WHERE  Orders.ShipCountry = 'France'
AND YEAR(Orders.OrderDate) = 1998
jjbyhhgthfiauklML; NUIHD