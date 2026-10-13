# Build BeatLeader's real RatingAPI with its analyzer/parser git submodules.
# The source is fetched at build time; pin this to a reviewed commit for production.
FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
RUN apt-get update && apt-get install -y --no-install-recommends git ca-certificates && rm -rf /var/lib/apt/lists/*
RUN git clone --recurse-submodules https://github.com/BeatLeader/RatingAPI.git /src
WORKDIR /src
COPY Program.cs /tmp/Program.cs
RUN cp /tmp/Program.cs /src/Program.cs
RUN dotnet publish RatingAPI.csproj -c Release -o /out

FROM mcr.microsoft.com/dotnet/aspnet:10.0
WORKDIR /app
COPY --from=build /out ./
ENV ASPNETCORE_URLS=http://+:8080
EXPOSE 8080
ENTRYPOINT ["dotnet", "RatingAPI.dll"]
