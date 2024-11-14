## Contribution Guidelines

- Fetch the latest changes and create a new branch for your feature/fix

```bash
git checkout main
git pull
git checkout -b your-branch-name
```

- As you're working, regularly fetch and merge other people's changes:

```bash
git fetch origin
git merge origin/main
```

- Make sure the project builds and tests pass
- Run linting on the codebase locally before pushing
- Push your branch and create a pull request against `main`

```bash
git push origin your-branch-name
```

- Follow through with CI checks and code reviews
